#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;
using namespace boost::asio;
namespace fs = std::filesystem;
constexpr std::string_view version = "0.0.2";
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
    std::ostringstream out;
    sock.connect(ep);
    std::ifstream fin(path);
    std::string text{std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};
    pt::ptree json_data;
    json_data.put("type","file");
    json_data.put("name",fs::path(path).filename().string());
    json_data.put("text",text);
    pt::write_json(out,json_data);
    char ans[1024];
    sock.write_some(buffer(out.str()+char(-1)));
    //char ans[1024];
    size_t bytes = read(sock, boost::asio::buffer(ans), boost::bind(read_complete,ans,_1,_2));
    return {ans,bytes-1};
}
int main () {
    //const std::string host = "streetms.ru";
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