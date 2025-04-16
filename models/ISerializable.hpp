#pragma once

namespace nx
{
  template <typename TEnum>
  struct ISerializable
  {
    virtual ~ISerializable() = default;

    [[nodiscard]]
    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize(const nlohmann::json& j) = 0;

    [[nodiscard]]
    virtual TEnum getType() const = 0;
  };
}