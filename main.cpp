#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
struct File{
    File(std::string_view name, std::string_view text) : name_(name), text_(text){};
    std::string name_;
    std::string text_;
};
size_t read_complete(char * buf, const boost::system::error_code & err, size_t bytes)
{
    if ( err)
        return 0;
    bool found = std::find(buf, buf + bytes, -1) < buf + bytes;
    return not found;
}
std::ostream& operator<<(std::ostream& out,File& file) {
    return out << file.name_ << "\n" << file.text_ << std::endl;
}
using namespace boost::asio;
namespace fs = std::filesystem;
constexpr std::string_view version = "0.0.2";

std::string send_file_to_server(const std::string& path, const ip::tcp::endpoint& ep)
{
    io_service service;
    ip::tcp::socket sock(service);
    boost::asio::streambuf buffer;
    std::ostream out(&buffer);
    sock.connect(ep);
    std::ifstream fin(path);
    std::string text{std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};
    text += char(-1);
    File file(fs::path(path).filename().string()+char(-1),text);
    out << file;
    boost::asio::write(sock,buffer);
    char ans[1024];
    size_t bytes = read(sock, boost::asio::buffer(ans), boost::bind(read_complete,ans,_1,_2));
    return {ans,bytes-1};
}
int main () {
    const std::string host = "localhost";
    const uint port = 8002;
#ifdef WIN32
    system("chcp 65001");
#endif
    std::string path;
    io_service service;
    ip::tcp::resolver resolver(service);
    ip::tcp::endpoint ep;
    try {
        ep = *resolver.resolve(ip::tcp::resolver::query(host, "8002"));
    }
    catch (std::exception& ex) {
        std::cout << "не получилось установить соединение с сервером. Убедитесь, что вы подключены к интернету";
        std::exit(1);
    }
    while (true) {
        std::cout << "\nвведите имя файла : ";
        std::cin >> path;
        if (fs::is_regular_file(path)) {
            std::cout << "тестирование...\n";
            try {
                std::cout << send_file_to_server(path,ep);
            } catch (boost::wrapexcept<boost::system::system_error>& ex) {
                std::cout << "потеряно соединение с сервером\nУбедитесь, что вы подключены к интернету\n";
            }
        } else {
            std::cout << "файл не найден\n";
        }
    }
}