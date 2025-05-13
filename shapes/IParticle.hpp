#pragma once

namespace nx
{

  struct IParticle : public sf::Drawable,
                     public sf::Transformable
  {

    // clones the particle and gives it the associated timestamp
    virtual IParticle * clone( const float timeStampInSeconds ) const = 0;

    virtual bool hasExpired() const = 0;

    /// @return [0 - 1) indicates it's alive, >= 1 indicates it's expired. < 0 indicates a bug.
    virtual float getTimeRemainingPercentage() const = 0;

    virtual float getTimeAliveInSeconds() const = 0;

    // used for updating the expiration time
    virtual void update( const sf::Time& deltaTime ) = 0;

    // this is automatically created
    virtual float getSpawnTimeInSeconds() const = 0;

    // the spawn time should be automatically created and this function is
    // used to extend the life of the particle by downstream modifiers
    virtual void setSpawnTimeInSeconds( float newSpawnTime ) = 0;

    // the expiration time is spawn time + time in seconds, so it must be >= spawn time
    virtual float getExpirationTimeInSeconds() const = 0;

    // the expiration time is the spawn time + time in seconds, and it is
    // originally set by the IParticleGenerator
    virtual void setExpirationTimeInSeconds( float expirationTime ) = 0;

    virtual uint8_t getPointCount() const = 0;

    virtual float getOutlineThickness() const = 0;

    virtual float getRadius() const = 0;

    virtual sf::FloatRect getLocalBounds() const = 0;
    virtual sf::FloatRect getGlobalBounds() const = 0;

    // virtual void fadeFillColors( const float percentage ) = 0;
    // virtual void fadeOutlineColors( const float percentage ) = 0;

    virtual void setColorPattern( const sf::Color & startColor, const sf::Color & endColor ) = 0;
    virtual std::pair< sf::Color, sf::Color > getColors() const = 0;

    virtual void setOutlineColorPattern( const sf::Color & startColor, const sf::Color & endColor ) = 0;
    virtual std::pair< sf::Color, sf::Color > getOutlineColors() const = 0;
  };
}