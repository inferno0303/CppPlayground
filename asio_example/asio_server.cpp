#include <iostream>
#include "../include/boost/asio.hpp"


class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::ip::tcp::socket socket)
        : socket_(std::move(socket)) {
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](const boost::system::error_code ec, const std::size_t length) {
                                    if (!ec) {
                                        std::cout << "[received] " << data_ << std::endl;
                                        do_write(length);
                                    }
                                });
    }

    void do_write(const std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                          [this, self](const boost::system::error_code ec, std::size_t /*length*/) {
                              if (!ec) {
                                  do_read();
                              }
                          });
    }

    boost::asio::ip::tcp::socket socket_;

    enum { max_length = 1024 };

    char data_[max_length]{};
};

class Server {
public:
    Server(boost::asio::io_context& io_context, const short port)
        : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                do_accept();
            });
    }

    boost::asio::ip::tcp::acceptor acceptor_;
};

int main() {
    try {

        boost::asio::io_context io_context;

        Server s(io_context, 9800);

        io_context.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
