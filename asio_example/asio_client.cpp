#include <iostream>
#include <thread>

#include "../include/boost/asio.hpp"

class TcpClient {
public:
    TcpClient(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : socket_(io_context), io_context_(io_context), is_running_(true) {
        boost::asio::ip::tcp::resolver resolver(io_context);
        const auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket_, endpoints);
    }

    void start() {
        // 启动接收线程
        receive_thread_ = std::thread(&TcpClient::receive_worker, this);

        // 启动发送线程
        send_thread_ = std::thread(&TcpClient::send_worker, this);

        // 等待所有线程完成
        if (receive_thread_.joinable()) receive_thread_.join();
        if (send_thread_.joinable()) send_thread_.join();

    }

    void stop() {
        is_running_ = false;
        io_context_.stop();
        if (send_thread_.joinable()) send_thread_.join();
        if (receive_thread_.joinable()) receive_thread_.join();
    }

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context& io_context_;
    std::thread send_thread_;
    std::thread receive_thread_;
    std::atomic<bool> is_running_;

    void receive_worker() {
        try {
            while (is_running_) {
                char buf[1024];
                boost::system::error_code error;
                const size_t len = socket_.read_some(boost::asio::buffer(buf), error);
                if (error) {
                    std::cerr << "Receive error: " << error.message() << std::endl;
                    break;
                }
                std::cout << "Received: " << std::string(buf, len) << std::endl;
            }
        } catch (std::exception& e) {
            std::cerr << "Receive exception: " << e.what() << std::endl;
        }
    }

    void send_worker() {
        try {
            while (is_running_) {
                std::ostringstream oss;
                oss << "Message from thread id: " << std::this_thread::get_id() << " , msg: Hello World. EOF";
                std::string message = oss.str();
                for (unsigned int i = 0; i < 5; ++i) {
                    write(socket_, boost::asio::buffer(message));
                }
                std::cout << "Sent: " << message << " x5" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // 定时发送消息
            }
        } catch (std::exception& e) {
            std::cerr << "Send exception: " << e.what() << std::endl;
        }
    }
};


int main() {
    try {
        boost::asio::io_context io_context;
        TcpClient client(io_context, "127.0.0.1", "9800");

        client.start();

        // 运行io_context来处理异步操作
        io_context.run();

        // 停止客户端并等待线程完成
        client.stop();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
