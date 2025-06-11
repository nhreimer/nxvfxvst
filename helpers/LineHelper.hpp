#pragma once

namespace nx
{

  /// Helps with setting the colors
  struct LineHelper
  {
    /////////////////////////////////////////////////////////
    /// updates based on particle colors
    static void updateLineColors( CurvedLine *line,
                             const IParticle *particleA,
                             const IParticle *particleB,
                             const bool invertPercentage )
    {
      const float percentageA =
      (invertPercentage) ? particleA->getTimeRemainingPercentage() : 1.f - particleA->getTimeRemainingPercentage();

      const float percentageB =
      (invertPercentage) ? particleB->getTimeRemainingPercentage() : 1.f - particleB->getTimeRemainingPercentage();

      // get the colors
      const auto colorsA = particleA->getColors();
      const auto colorsB = particleB->getColors();

      // get the mixed colors for each particle at this exact moment
      const auto lerpedColorA = ColorHelper::lerpColor(colorsA.first, colorsA.second, percentageA);
      const auto lerpedColorB = ColorHelper::lerpColor(colorsB.first, colorsB.second, percentageB);

      // now mix those mixed colors for the line
      line->setGradient(ColorHelper::getColorPercentage(lerpedColorA, percentageA, false ),
                        ColorHelper::getColorPercentage(lerpedColorB, percentageB, false ) );
    }

    /////////////////////////////////////////////////////////
    /// updates based on custom colors
    static void updateCustomLineColors( CurvedLine * line,
                                 const IParticle * particleA,
                                 const IParticle * particleB,
                                 const sf::Color& lineColorA,
                                 const sf::Color& lineColorB,
                                 const bool invertPercentage )
    {
      const float percentageA =
      (invertPercentage) ? particleA->getTimeRemainingPercentage() : 1.f - particleA->getTimeRemainingPercentage();

      const float percentageB =
      (invertPercentage) ? particleB->getTimeRemainingPercentage() : 1.f - particleB->getTimeRemainingPercentage();

      // get the mixed colors for each particle at this exact moment
      const auto lerpedColorA = ColorHelper::lerpColor(lineColorA, lineColorB, percentageA);
      const auto lerpedColorB = ColorHelper::lerpColor(lineColorA, lineColorB, percentageB);

      // now mix those mixed colors for the line
      line->setGradient(ColorHelper::getColorPercentage(lerpedColorA, percentageA, false ),
                        ColorHelper::getColorPercentage(lerpedColorB, percentageB, false ) );
    }
  };

}