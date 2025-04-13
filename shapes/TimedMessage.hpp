#pragma once

namespace nx
{
  class TimedMessage
  {

  public:
    void setMessage( std::string&& msg, int32_t timeoutInSeconds = 5 )
    {
      m_message = msg;
      m_messageClock.restart();
    }

    [[nodiscard]]
    bool hasExpired() const
    {
      return m_messageClock.getElapsedTime().asSeconds() >= static_cast< float >( m_timeoutInSeconds );
    }

    [[nodiscard]]
    std::string_view getMessage() const { return m_message; }

  private:
    std::string m_message;
    sf::Clock m_messageClock;
    int32_t m_timeoutInSeconds { 5 };
  };
}