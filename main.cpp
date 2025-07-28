/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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