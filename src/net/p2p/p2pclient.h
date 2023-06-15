#ifndef P2PCLIENT_H
#define P2PCLIENT_H

#include <memory>

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/websocket.hpp>

#include "../../libs/BS_thread_pool_light.hpp"
#include "../../utils/utils.h"
#include "p2pbase.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace P2P {
  // Forward declarations.
  class ManagerBase;
  class Message;

  /// Abstraction of a client-specific connection/session.
  class ClientSession : public BaseSession, public std::enable_shared_from_this<ClientSession> {
    private:
      /// TCP resolver, for resolving endpoints.
      tcp::resolver resolver_;

      /// Dedicated buffer for read operations.
      beast::flat_buffer receiveBuffer_;

      /// Dedicated buffer for write operations.
      beast::flat_buffer answerBuffer_;

      /// Mutex for managing read/write access to the write buffer.
      std::mutex writeLock_;

      /// Object that defines the type of the response.
      boost::beast::websocket::response_type req_;

    public:
      /**
       * Constructor (for BaseSession).
       * @param ioc The I/O core object.
       * @param host The target host IP/address to connect to.
       * @param port The target host port to connect to.
       * @param manager Reference to the connection manager.
       * @param threadPool Reference pointer to the thread pool.
       */
      ClientSession(
        net::io_context& ioc, const std::string& host,
        const unsigned short& port, ManagerBase& manager,
        const std::unique_ptr<BS::thread_pool_light>& threadPool
      ) : BaseSession(ioc, manager, host, port, ConnectionType::CLIENT, threadPool),
      resolver_(net::make_strand(ioc)) {}

      /// Startup the client. Implementation overriden from BaseSession.
      void run() override;

      /// Shutdown the client. Implementation overriden from BaseSession.
      void stop() override;

      /// Resolve an endpoint.
      void resolve();

      /**
       * Callback for the resolve operation.
       * @param ec The error code to parse, if necessary.
       * @param results The resolve's result object.
       */
      void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

      /**
       * Connect to an endpoint.
       * @param results The resolved endpoint generated by resolve().
       */
      void connect(tcp::resolver::results_type& results);

      /**
       * Callback for the connect operation.
       * @param ec The error code to parse, if necessary.
       * @param ep The resulting endpoint type object.
       */
      void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

      /**
       * Give a handshake to the endpoint.
       * @param host The host to give a handshake to.
       */
      void handshake(const std::string& host);

      /// Do a read operation. Implementation overriden from BaseSession.
      void read() override;

      /**
       * Callback for read operation. Implementation overriden from BaseSession.
       * @param ec The error code to parse if necessary.
       * @param bytes_transferred The number of bytes transferred in the operation.
       */
      void on_read(beast::error_code ec, std::size_t bytes_transferred) override;

      /// Do a write operation. Implementation overriden from BaseSession.
      void write(const Message& data) override;

      /**
       * Callback for write operation. Implementation overriden from BaseSession.
       * @param ec The error code to parse if necessary.
       * @param bytes_transferred The number of bytes transferred in the operation.
       */
      void on_write(beast::error_code ec, std::size_t bytes_transferred) override;

      /// Close the websocket connection. Implementation overriden from BaseSession.
      void close() override;

      /**
       * Callback for close operation. Implementation overriden from BaseSession.
       * @param ec The error code to parse if necessary.
       */
      void on_close(beast::error_code ec) override;

      /**
       * Handle an error and close the connection.
       * @param func The name of the function where the error occurred.
       * @param ec The error code object to parse.
       */
      void handleError(const std::string& func, const beast::error_code& ec);
  };
}

#endif  // P2PCLIENT_H
