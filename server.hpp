
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>

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

public:
    static pointer create(boost::asio::io_service& io_service, ThreadPool& workers) {
        return pointer(new TCPConnection(io_service, workers));
    }

    void start()
    {
        // read some
        // call processing and read some
        //auto this_ptr = shared_from_this();
        workers_.enqueue([](const pointer& this_ptr) {
            std::string message = "test";
            std::cerr << "use counter: " << this_ptr.use_count();
            boost::asio::async_write(this_ptr->socket_, boost::asio::buffer(message),
                                     boost::bind(&TCPConnection::handle_write, this_ptr));
            std::cerr << "going to sleep\n";
            sleep(5);
            std::cerr << "woke up\n";

            message = "=res";
            boost::asio::async_write(this_ptr->socket_, boost::asio::buffer(message),
                                     boost::bind(&TCPConnection::handle_write, this_ptr));
        }, shared_from_this());
    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }
private:
    TCPConnection(boost::asio::io_service& io_service, ThreadPool& workers)
            : socket_(io_service), workers_(workers) {
    }

    void handle_write() {
        std::cout << "written\n";
        //boost::asio::async_read(socket_, );
    }

    boost::asio::ip::tcp::socket socket_;
    ThreadPool& workers_;
};

class TCPServer
{
public:
    TCPServer(const char* ip, unsigned port, unsigned bg_workers_number)
            : end_point_(boost::asio::ip::address::from_string(ip), port)
            , acceptor_(service_, end_point_), workers_(bg_workers_number) {
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        start_accept();
    }
    void run() {
        service_.run();
    }

private:
    void start_accept() {
        TCPConnection::pointer new_connection =
                TCPConnection::create(acceptor_.get_io_service(), workers_);

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
};

