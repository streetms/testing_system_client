#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
using namespace boost::asio;
constexpr std::string_view version = "0.0.0";
size_t read_complete(char * buf, const boost::system::error_code & err, size_t bytes)
{
    if ( err)
        return 0;
    bool found = std::find(buf, buf + bytes, -1) < buf + bytes;
    return not found;
}
std::string send_file_to_server(const std::string& path, const ip::tcp::endpoint& ep)
{
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);
    std::ifstream fin(path);
    sock.write_some(buffer(path+char(-1)));
    std::string text{std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};
    text += char(-1);
    sock.write_some(buffer(text));
    char buf[1024];
    size_t bytes = read(sock, buffer(buf), boost::bind(read_complete,buf,_1,_2));
    std::string ans(buf,bytes-1);
    return ans;
}
int main (int argc, char* argv[]) {
    std::string path;
    io_service service;
    ip::tcp::resolver resolver(service);
    ip::tcp::endpoint ep = *resolver.resolve(ip::tcp::resolver::query ("streetms.ru", "8002"));
    while (true) {
        std::cout << "введите имя файла : ";
        std::cin >> path;
        if (std::filesystem::is_regular_file(path)) {
            std::cout << "тестирование...\n";
            try {
                std::cout << send_file_to_server(path,ep);
            } catch (boost::wrapexcept<boost::system::system_error>& ex) {
                std::cout << "потеряно соединение с сервером\n";
            }
        } else {
            std::cout << "файл не найден\n";
        }
    }
}