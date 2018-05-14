
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <boost/array.hpp>
#include "thread_pool.hpp"

template <class TProcessor>
class StreamsProcessor {
public:
    StreamsProcessor(TProcessor&& processor) : processor_(std::move(processor)) {

    }
    // streams to extend upto multiline evalutations
    void process(std::istream& is, std::ostream& os) {
        std::string line;
        while(getline(is, line)) {
            os << processor_.process(line) << std::endl;
        }
    }
private:
    TProcessor processor_;
};



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
        std::cerr << "start retreiving data at " << std::time(0) << "\n";
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                        boost::bind(&TCPConnection::handle_read, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));

    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }
private:
    TCPConnection(boost::asio::io_service& io_service, ThreadPool& workers, Processor& processor)
            : socket_(io_service)
            , workers_(workers)
            , processor_(processor) {
    }

    void handle_write(const std::shared_ptr<std::string>& /*message_ptr*/) {
        //std::cout << "written " << *message_ptr << "\n";
    }

    void handle_read(const boost::system::error_code& e, const std::size_t bytes_transferred) {
        //std::cerr << "Got msg sz: " << bytes_transferred << "msg:" << std::string(buffer_.begin(), buffer_.begin() + bytes_transferred) << "\n";
        if (!e || e == boost::asio::error::eof) {
            std::vector<std::string> reqs;
            long long enter_character_ind = -1;
            long i = 0;
            while (i < static_cast<long>(bytes_transferred)) {
                if (buffer_[i] == '\n' || buffer_[i] == '\r') {
                    if (enter_character_ind + 1 != i || enter_character_ind == -1) {
                        reqs.emplace_back(std::string(&buffer_[enter_character_ind + 1], &buffer_[i]));
                    }
                    enter_character_ind = i;
                } else if (!std::isprint(buffer_[i])) {
                    std::cerr << "Error non printable character code " << int(buffer_[i]) << "\n";
                    return;
                }
                ++i;
            }
            if (e == boost::asio::error::eof) {
                reqs.emplace_back(std::string(buffer_.begin() + (enter_character_ind + 1), buffer_.begin() + bytes_transferred));
                enter_character_ind = bytes_transferred - 1;
            }
            if (!reqs.empty()) {
                expression_ += reqs[0];
                reqs[0] = std::move(expression_);
            }
            std::copy(buffer_.begin() + (enter_character_ind + 1), buffer_.begin() + bytes_transferred,
                      std::back_inserter(expression_));
            //std::cerr << "exp_rest:" << expression_ << "\n";
            process(std::move(reqs), e == boost::asio::error::eof);
        } else {
            std::cerr << "error: "<< e.message() << "\n";
        }
        // destroy object at the end
    }

    void process(std::vector<std::string>&& vec_messages, bool stop = false) {
        workers_.enqueue([stop](const pointer& this_ptr, const std::vector<std::string>& vec_messages) {
            for (const auto& message: vec_messages) {
                std::shared_ptr<std::string> res = std::make_shared<std::string>(this_ptr->processor_(message) + "\n");
                boost::asio::async_write(this_ptr->socket_, boost::asio::buffer(*res),
                                         boost::bind(&TCPConnection::handle_write, this_ptr, res));
            }
            if (stop) {
                return;
            }
            this_ptr->socket_.async_read_some(boost::asio::buffer(this_ptr->buffer_),
                                              boost::bind(&TCPConnection::handle_read, this_ptr,
                                                          boost::asio::placeholders::error,
                                                          boost::asio::placeholders::bytes_transferred));
        }, shared_from_this(), std::move(vec_messages));
    }

    boost::asio::ip::tcp::socket socket_;
    ThreadPool& workers_;
    boost::array<char, 8192> buffer_;
    std::string expression_;
    Processor& processor_;
    std::string output_buffer_;
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

