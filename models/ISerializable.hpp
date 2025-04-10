#pragma once

namespace nx
{
  template < typename T >
  struct ISerializable
  {
    virtual ~ISerializable() = default;

    [[nodiscard]]
    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize( const nlohmann::json& j ) = 0;

    // identify type for easier loading
    [[nodiscard]]
    virtual T getType() const = 0;
  };
}