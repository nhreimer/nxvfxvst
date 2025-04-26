#pragma once

namespace nx
{

  class MenuHelper
  {
    public:

    static void drawBlendOptions( sf::BlendMode& blendMode )
    {
      ImGui::PushID( &blendMode );
      {
        blendMode = addBlendOption( "none", blendMode, sf::BlendNone );
        ImGui::SameLine();
        blendMode = addBlendOption( "add", blendMode, sf::BlendAdd );
        ImGui::SameLine();
        blendMode = addBlendOption( "alpha", blendMode, sf::BlendAlpha );
        ImGui::SameLine();
        blendMode = addBlendOption( "multiply", blendMode, sf::BlendMultiply );
        ImGui::SameLine();
        blendMode = addBlendOption( "min", blendMode, sf::BlendMin );
        ImGui::SameLine();
        blendMode = addBlendOption( "max", blendMode, sf::BlendMax );
      }
      ImGui::PopID();
    }

    static sf::BlendMode addBlendOption(
                                   const std::string & label,
                                   sf::BlendMode & blendOption,
                                   const sf::BlendMode & newBlendOption )
    {
      // set up checkmark beforehand for user feedback
      bool isBlend = ( blendOption == newBlendOption );
      if ( ImGui::Checkbox( label.c_str(), &isBlend ) )
        blendOption = newBlendOption;

      return blendOption;
    }
 };

}