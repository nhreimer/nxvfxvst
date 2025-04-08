#include <SFML/Graphics.hpp>

#include "log/Logger.hpp"
#include "app/EventFacadeApp.hpp"

void run()
{
  nx::SLog::initializeConsole();

  sf::RenderWindow window(
    sf::VideoMode( { 1280, 768 } ),
    "Test Window",
    sf::Style::Default,
    sf::State::Windowed,
    sf::ContextSettings(
      24, 0, 8, 4, 6, sf::ContextSettings::Default, false ) );

  window.setFramerateLimit( 60 );

  nx::EventFacadeApp eventFacade;
  eventFacade.initialize( window );

  while ( window.isOpen() )
  {
    eventFacade.executeFrame( window );
  }
  eventFacade.shutdown( window );
}

int main()
{
  run();
  return 0;
}