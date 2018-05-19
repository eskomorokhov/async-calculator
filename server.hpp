#pragma once

#include <malloc.h>

#include <deque>
#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

#include "log.hpp"
#include "thread_pool.hpp"

namespace IO {

class TCPConnection: public std::enable_shared_from_this<TCPConnection>
{
public:
    typedef std::shared_ptr<TCPConnection> pointer;
    typedef std::function<std::string(const std::string&)> Processor;

public:
    static pointer create(
            boost::asio::io_service& io_service,
            ThreadPool& workers,
            Processor& processor) {
        return pointer(new TCPConnection(io_service, workers, processor));
    }

    void start()
    {
        log_info("start retreiving data at ", std::time(0));
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                        boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));

    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }
    virtual ~TCPConnection() {
        log_info("Finished connection at ", std::time(0));
    }
private:
    TCPConnection(boost::asio::io_service& io_service, ThreadPool& workers, Processor& processor)
            : socket_(io_service)
            , workers_(workers)
            , processor_(processor)
            , is_calculating_(ATOMIC_FLAG_INIT) {
    }
    void handle_write(const std::shared_ptr<std::string>& /*message_ptr*/) {
        //std::cout << "written " << *message_ptr << "\n";
        bool expected = false;
        if (!expressions_.empty() && is_calculating_.compare_exchange_strong(expected, true,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
            process(std::move(expressions_));
        }
    }

    void handle_read(const boost::system::error_code& e, const std::size_t bytes_transferred) {
        log_debug("Got msg sz: ", bytes_transferred, "msg:", std::string(buffer_.begin(), buffer_.begin() + bytes_transferred));
        if (!e || e == boost::asio::error::eof) {
            std::vector<std::deque<char> > reqs;
            long long enter_character_ind = -1;
            long i = 0;
            while (i < static_cast<long>(bytes_transferred)) {
                if (buffer_[i] == '\n' || buffer_[i] == '\r') {
                    if (enter_character_ind + 1 != i || enter_character_ind == -1) {
                        reqs.emplace_back(std::deque<char>(&buffer_[enter_character_ind + 1], &buffer_[i]));
                    }
                    enter_character_ind = i;
                }
                // ignore due to performance issues
                //else if (!std::isprint(buffer_[i])) {
                //    std::cerr << "Error non printable character code " << int(buffer_[i]) << "\n";
                //    return;
                //}
                ++i;
            }
            if (e == boost::asio::error::eof) {
                reqs.emplace_back(std::deque<char>(buffer_.begin() + (enter_character_ind + 1), buffer_.begin() + bytes_transferred));
                enter_character_ind = bytes_transferred - 1;
            }
            if (!reqs.empty()) {
                std::copy(reqs[0].begin(), reqs[0].begin() + reqs[0].size(), std::back_inserter(pending_expression_));
                reqs[0] = std::move(pending_expression_);
            }
            std::copy(buffer_.begin() + (enter_character_ind + 1), buffer_.begin() + bytes_transferred,
                      std::back_inserter(pending_expression_));
            //std::cerr << "exp_rest:" << pending_expression_ << "\n";
            for (auto& expression: reqs) {
                expressions_.push_back(std::move(expression));
            }
            bool expected = false;
            if (!expressions_.empty() && is_calculating_.compare_exchange_strong(expected, true,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
                    process(std::move(expressions_));
            }
            if (e != boost::asio::error::eof) {
                socket_.async_read_some(boost::asio::buffer(buffer_),
                                              boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                                          boost::asio::placeholders::error,
                                                          boost::asio::placeholders::bytes_transferred));
            }
        } else {
            log_debug("error: ", e.message());
        }
        // destroy object at the end
    }

    void process(std::vector<std::deque<char> >&& vec_messages) {
        workers_.enqueue([](const std::weak_ptr<TCPConnection>& this_weak_ptr, std::vector<std::deque<char> > vec_messages) {
            for (auto& message: vec_messages) {
                auto this_ptr = this_weak_ptr.lock();
                if (!this_ptr) {
                    log_info("Interrupt calculation due to died connection");
                    break;
                }
                std::string exp;
                exp.append(message.begin(), message.begin() + message.size());
                message.clear();
                malloc_trim(0);
                std::shared_ptr<std::string> res = std::make_shared<std::string>(this_ptr->processor_(exp) + "\n");
                boost::asio::async_write(this_ptr->socket_, boost::asio::buffer(*res),
                                         boost::bind(&TCPConnection::handle_write, this_ptr, res));
            }
            if (auto this_ptr = this_weak_ptr.lock()) {
                bool expected = true;
                this_ptr->is_calculating_.compare_exchange_strong(expected, false,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
                std::shared_ptr<std::string> res = std::make_shared<std::string>();
                boost::asio::async_write(this_ptr->socket_, boost::asio::buffer(*res),
                                         boost::bind(&TCPConnection::handle_write, this_ptr, res));
            }
        }, std::weak_ptr<TCPConnection>(shared_from_this()), std::move(vec_messages));
    }

    boost::asio::ip::tcp::socket socket_;
    ThreadPool& workers_;
    boost::array<char, 8192> buffer_;
    std::deque<char> pending_expression_;
    std::vector<std::deque<char>> expressions_;
    Processor& processor_;
    std::deque<char> output_buffer_;
    std::atomic<bool> is_calculating_;
};

class TCPServer
{
    typedef std::function<std::string(const std::string&)> Processor;
public:
    TCPServer(const char* ip, unsigned port, unsigned bg_workers_number, Processor processor)
            : end_point_(boost::asio::ip::address::from_string(ip), port)
            , acceptor_(service_, end_point_), workers_(bg_workers_number)
            , processor_(processor) {
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        start_accept();
    }
    void run() {
        service_.run();
    }

private:
    void start_accept() {
        TCPConnection::pointer new_connection =
                TCPConnection::create(acceptor_.get_io_service(), workers_, processor_);

        acceptor_.async_accept(new_connection->socket(),
               boost::bind(&TCPServer::handle_accept, this, new_connection, boost::asio::placeholders::error));
    }

    void handle_accept(TCPConnection::pointer new_connection,
                       const boost::system::error_code& error) {
        if (!error) {
            new_connection->start();
        }
        start_accept();
    }

    boost::asio::io_service service_;
    boost::asio::ip::tcp::endpoint end_point_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ThreadPool workers_;
    Processor processor_;
};

}   //  namespace IO