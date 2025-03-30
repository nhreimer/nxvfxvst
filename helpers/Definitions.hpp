#pragma once

#include <deque>

#include "data/DataObjects.hpp"

namespace nx
{
  struct MidiVisNode_t;

  constexpr int32_t MAX_CHANNELS = 16;

  template < typename T >
  using ChannelData = std::array< T, MAX_CHANNELS >;

  using ChannelParticles = std::array< std::deque< MidiVisNode_t >, MAX_CHANNELS >;

}