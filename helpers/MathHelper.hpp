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

#pragma once

namespace nx
{

constexpr float NX_PI   = 3.1415926535897f;
constexpr float NX_R2D  = 57.29577951308f;
constexpr float NX_TAU  = 6.283185307f;
constexpr float NX_D2R  = 0.017453293f;

  struct LineEquation_t
  {
    LineEquation_t()
      : m( 0.f ), b( 0.f )
    {}

    float m;
    float b;
  };

  struct MathHelper
  {

    static sf::Vector2f polarToCartesian( const float angleDeg, const float len )
    {
      const float angleRad = angleDeg * NX_D2R;
      return sf::Vector2f { std::cos( angleRad ), std::sin( angleRad ) } * len;
    }

    template < typename T >
    static T roundTo( T value, const int places )
    {
      auto p = std::pow( 10, places );
      return static_cast< T >( std::round( value * p ) / p );
    }

    static float getDistance( const sf::Vector2f& pointA, const sf::Vector2f& pointB )
    {
      return std::sqrt( ( pointB.x - pointA.x ) * ( pointB.x - pointA.x ) +
                        ( pointB.y - pointA.y ) * ( pointB.y - pointA.y ) );
    }

    static LineEquation_t getLineEquation( const sf::Vector2f& pointA,
                                           const sf::Vector2f& pointB )
    {
      LineEquation_t result;

      result.m = ( pointB.y - pointA.y ) / ( pointB.x - pointA.x );

      // plug in x and y then solve for b
      result.b = -pointA.y + result.m * pointA.x;

      return result;
    }

    static float getAngleOfLineInRadians( const LineEquation_t& lineEquation )
    {
      return atan( lineEquation.m );
    }

    static float getAngleOfLineInDegrees( const LineEquation_t& lineEquation )
    {
      return static_cast< float >( getAngleOfLineInRadians( lineEquation ) * 180.f / NX_PI );
    }

    static float getPerpendicularSlope( const sf::Vector2f& pointA,
                                        const sf::Vector2f& pointB )
    {
      return - ( pointB.x - pointA.x ) / ( pointB.y - pointA.y );
    }

    /***
     *
     * @param partitions the total number of segments around a circle
     * @param partitionIndex the index around the circle needed
     * @param xDistance distance from center (1 = default)
     * @param yDistance distance from center (1 = default)
     * @return x and y in degrees
     */
    static sf::Vector2f getAnglePosition( const uint32_t partitions,
                                          const uint32_t partitionIndex,
                                          const float xDistance = 1.f,
                                          const float yDistance = 1.f )
    {
      const float anglePartitions = NX_TAU / static_cast< float >( partitions );
      const float angle = static_cast< float >( anglePartitions ) * static_cast< float >( partitionIndex );
      return sf::Vector2f { std::cos( angle ) * NX_R2D * xDistance, std::sin( angle ) * NX_R2D * yDistance };
    }

    static sf::Vector2f getPositionFromNote( const int32_t noteNumber,
                                             const int32_t octave,
                                             const float octaveAmplifier )
    {
      const float angle = static_cast< float >(noteNumber) / 12.f;
      return sf::Vector2f { std::cos( angle ) * static_cast< float >(octave) * octaveAmplifier,
                            std::sin( angle ) * static_cast< float >(octave) * octaveAmplifier };
    }

  };

}