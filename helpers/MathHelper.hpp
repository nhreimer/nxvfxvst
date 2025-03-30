#pragma once

namespace nx
{

constexpr float POLY_PI   = 3.1415926535897f;
constexpr float POLY_R2D  = 57.29577951308f;
constexpr float POLY_TAU  = 6.283185307f;
constexpr float POLY_D2R  = 0.017453293f;

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
    template < typename T >
    static T roundTo( T value, int places )
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
      return static_cast< float >( getAngleOfLineInRadians( lineEquation ) * 180.f / POLY_PI );
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
    static sf::Vector2f getAnglePosition( uint32_t partitions,
                                          uint32_t partitionIndex,
                                          float xDistance = 1.f,
                                          float yDistance = 1.f )
    {
      const float anglePartitions = POLY_TAU / static_cast< float >( partitions );
      const float angle = static_cast< float >( anglePartitions ) * static_cast< float >( partitionIndex );
      return sf::Vector2f { std::cos( angle ) * POLY_R2D * xDistance, std::sin( angle ) * POLY_R2D * yDistance };
    }

    static sf::Vector2f getPositionFromNote( int32_t noteNumber,
                                             int32_t octave,
                                             float octaveAmplifier )
    {
      float angle = ( float )noteNumber / 12.f;
      return sf::Vector2f { std::cos( angle ) * ( float )octave * octaveAmplifier,
                            std::sin( angle ) * ( float )octave * octaveAmplifier };
    }

    /***
     * A vertical partitioner that is very similar to bjorklund:
     * k = 5, n = 13 =>
     *  1   1   1   1   1
     *  0   0   0   0   0
     *  0   0
     *
     * which turns into
     *  1   1   1
     *  0   0   0
     *  0   0
     *  1   1
     *  0   0
     *
     *  It does this in O( k ) steps by pairing columns, e.g., 0 & k - 1, 0 & k - 2
     * @param k number of slots to be active
     * @param n number of slots total
     * @param rotate number of positions to offset
     * @return vector of bool values
     */
    static std::vector< bool > partition( int k, int n, int rotate = 0 )
    {
      int remainder = n % k;
      float gapRatio = static_cast< float >( n ) / static_cast< float >( k );

      int hardGapSize = static_cast< int >( gapRatio ) + 1;

      std::vector< bool > result( n );
      int y = 0;
      for ( int i = 0; i < remainder; ++i, y += hardGapSize + ( hardGapSize - 1 ) )
      {
        result[ ( y + rotate ) % n ] = true;

        if ( y + hardGapSize < n )
          result[ ( y + hardGapSize + rotate ) % n ] = true;
      }

      for ( int i = remainder; i < k - remainder; ++i, y += ( hardGapSize - 1 ) )
        result[ ( y + rotate ) % n ] = true;

      return result;
    }
  };

}