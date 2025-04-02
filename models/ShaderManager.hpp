#pragma once

namespace nx
{

  class ShaderManager final
  {
  public:

    explicit ShaderManager( const GlobalInfo_t& globalInfo )
      : m_globalInfo( globalInfo )
    {}


  private:
    const GlobalInfo_t& m_globalInfo;

    std::deque< std::unique_ptr< IShader > > m_shaders;
  };

}