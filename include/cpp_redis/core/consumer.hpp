// The MIT License (MIT)
//
// Copyright (c) 11/27/18 nick. <nbatkins@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.#ifndef CPP_REDIS_CONSUMER_HPP

#ifndef CPP_REDIS_CONSUMER_HPP
#define CPP_REDIS_CONSUMER_HPP

#include <cpp_redis/core/client.hpp>
#include <cpp_redis/misc/dispatch_queue.hpp>
#include <string>

namespace cpp_redis {

using defer = std::shared_ptr<void>;

#define READ_NEW ">"
#define CPP_REDIS_WILD_CARD "*"

//!
//!  reply callback called whenever a reply is received
//!  takes as parameter the received reply
//!
using consumer_callback_t = dispatch_callback_t;

struct consumer_callback_container {
  consumer_callback_t consumer_callback;
  acknowledgement_callback_t acknowledgement_callback;
};

using consumer_callback_container_t = consumer_callback_container;

struct consumer_reply {
  std::string group_id;
  xstream_reply_t reply;
};

using consumer_reply_t = consumer_reply;

class consumer_client_container {
public:
  consumer_client_container();

  client_t ack_client;
  client_t poll_client;
};

using consumer_client_container_t = consumer_client_container;

using client_container_ptr_t = std::unique_ptr<consumer_client_container_t>;

using consumer_callbacks_t =
    std::multimap<std::string, consumer_callback_container_t>;

// typedef std::map<std::string, consumer_callback_container_t>
// consumer_callbacks_t;

class consumer {
public:
  explicit consumer(
      std::string stream, std::string consumer,
      std::size_t max_concurrency = std::thread::hardware_concurrency());

  consumer &subscribe(
      const std::string &group, const consumer_callback_t &consumer_callback,
      const acknowledgement_callback_t &acknowledgement_callback = nullptr);

  //!
  //!  @brief Connect to redis server
  //!  @param host host to be connected to
  //!  @param port port to be connected to
  //!  @param connect_callback connect handler to be called on connect events
  //!  (may be null)
  //!  @param timeout_ms maximum time to connect
  //!  @param max_reconnects maximum attempts of reconnection if connection
  //!  dropped
  //!  @param reconnect_interval_ms time between two attempts of reconnection
  //!
  void connect(const std::string &host = "127.0.0.1", std::size_t port = 6379,
               const connect_callback_t &connect_callback = nullptr,
               int timeout_ms = 0, int max_reconnects = 0,
               int reconnect_interval_ms = 0);

  void auth(const std::string &password,
            const reply_callback_t &reply_callback = nullptr);

  //!
  //!  commit pipelined transaction
  //!  that is, send to the network all commands pipelined by calling send() /
  //!  subscribe() / ...
  //!
  //!  @return current instance
  //!
  consumer &commit();

  void dispatch_changed_handler(std::size_t size);

private:
  void poll();

private:

  client_container_ptr_t m_client;

  consumer_callbacks_t m_callbacks;
  std::mutex m_callbacks_mutex;

  dispatch_queue_ptr_t m_dispatch_queue;
  std::atomic_bool dispatch_queue_full{false};
  std::condition_variable dispatch_queue_changed;
  std::mutex dispatch_queue_changed_mutex;

  bool is_ready = false;
  std::atomic_bool m_should_read_pending{true};
  private:
  //!
  //!  the name of the stream
  //!
  std::string m_stream;

  //!
  //!  the name of this consumer group
  //!
  std::string m_name;

  //!
  //!  the topic ID
  //!
  std::string m_read_id;

  //!
  //!  number of milliseconds to block when polling
  //!
  int m_block_sec;

  //!
  //!  maximum number of worker threads
  //!
  std::size_t m_max_concurrency;
  
  //!
  //!  number of messages read
  //!
  int m_read_count;
};

using consumer_t = consumer;

} // namespace cpp_redis

#endif // CPP_REDIS_CONSUMER_HPP