#include "models/shader/easings/TimeEasing.hpp"

namespace nx
{

  nlohmann::json TimeEasing::serialize() const
  {
    return
    {
      { "easingType", SerialHelper::serializeEnum( m_data.easingType ) },
      { "decayRate", m_data.decayRate },
      { "intensity", m_data.intensity }
    };
  }

  void TimeEasing::deserialize( const nlohmann::json& j )
  {
    m_data.easingType = SerialHelper::deserializeEnum< E_TimeEasingType >( j.value( "easingType", "easeLinear" ) );
    m_data.decayRate = j.value( "decayRate", 0.15f );
    m_data.intensity = j.value( "intensity", 0.1f );
  }

  void TimeEasing::setEasingType( const E_TimeEasingType easingType )
  {
    m_data.easingType = easingType;
  }

  float TimeEasing::getDecayRate() const { return m_data.decayRate; }

  void TimeEasing::trigger()
  {
    // when using E_Time, do NOT reset the clock or else you'll get a linear-style effect
    m_timeTriggeredInSeconds = ( m_data.easingType == E_TimeEasingType::E_TimeContinuous )
      ? m_clock.getElapsedTime().asSeconds()
      : m_clock.restart().asSeconds();
  }

  float TimeEasing::getElapsedTime() const { return m_clock.getElapsedTime().asSeconds(); }

  TimeEasing::TimeEasingData_t& TimeEasing::getData() { return m_data; }

  void TimeEasing::setData( const TimeEasingData_t& data )
  {
    m_data = data;
    setEasingFunction( m_data.easingType );
  }

  [[nodiscard]]
  float TimeEasing::getEasing() const
  {
    const float decay = m_data.decayRate <= 0.001f ? 0.f : m_clock.getElapsedTime().asSeconds() / m_data.decayRate;

    switch ( m_data.easingType )
    {
      case E_TimeEasingType::E_Disabled:
        return 1.f;

      case E_TimeEasingType::E_Fixed:
        return m_data.intensity;

      case E_TimeEasingType::E_TimeContinuous:
      case E_TimeEasingType::E_TimeIntervallic:
        return m_clock.getElapsedTime().asSeconds();

      // use elapsed over decay by a multiplier
      case E_TimeEasingType::E_Impulse:
        return m_easingFunction( decay * m_data.intensity );

      case E_TimeEasingType::E_SparkleFlicker:
        return m_easingFunction( decay ) * m_data.intensity;

      default:
       return m_easingFunction( std::clamp(
         decay,
         0.f,
         1.f ) );
    }
  }

  void TimeEasing::drawMenu()
  {
    ImGui::PushID( this );

    ImGui::Text( "Time Easing Functions:" );

    if ( ImGui::BeginCombo( "##Easings", m_easingFunctionNames[ m_selectedComboIndex ] ) )
    {
      for ( int32_t i = 0; i < m_easingFunctionNames.size(); ++i )
      {
        const auto isSelected = ( m_selectedComboIndex == i );
        if ( ImGui::Selectable( m_easingFunctionNames[ i ], isSelected ) )
        {
          m_selectedComboIndex = i;
          m_data.easingType = m_easingTypes[ i ];
        }

        if ( isSelected )
          ImGui::SetItemDefaultFocus();
      }

      ImGui::EndCombo();
    }

    if ( m_data.easingType != E_TimeEasingType::E_Disabled )
    {
      ImGui::SliderFloat( "##DecayRate", &m_data.decayRate, 0.01f, 1.5f, "Decay Rate %0.2f seconds" );
      if ( m_data.easingType == E_TimeEasingType::E_SparkleFlicker )
        ImGui::SliderFloat( "##Sparkle Intensity", &m_data.intensity, 0.f, 3.f, "Intensity %0.2f" );
      else if ( m_data.easingType == E_TimeEasingType::E_Impulse || m_data.easingType == E_TimeEasingType::E_Fixed )
        ImGui::SliderFloat( "##Impulse Scale", &m_data.intensity, 0.f, 10.f, "Scale %0.2f" );
    }

    // if ( drawRadio( "Disabled", E_TimeEasingType::E_Disabled ) ) m_easingFunction = useNone;
    // else if ( drawRadio( "Fixed", E_TimeEasingType::E_Fixed ) ) m_easingFunction = useNone;
    // else if ( drawRadio( "Time Continuous", E_TimeEasingType::E_TimeContinuous, true ) ) m_easingFunction = useNone;
    // else if ( drawRadio( "Time Intervallic", E_TimeEasingType::E_TimeIntervallic, true ) ) m_easingFunction = useNone;
    //
    // else if ( drawRadio( "Linear", E_TimeEasingType::E_Linear ) ) m_easingFunction = easeOutLinear;
    // else if ( drawRadio( "Quadratic", E_TimeEasingType::E_Quadratic, true ) ) m_easingFunction = easeOutQuad;
    // else if ( drawRadio( "Cubic", E_TimeEasingType::E_Cubic, true ) ) m_easingFunction = easeOutCubic;
    // else if ( drawRadio( "Quartic", E_TimeEasingType::E_Quartic, true ) ) m_easingFunction = easeOutQuart;
    //
    // else if ( drawRadio( "Sine", E_TimeEasingType::E_Sine ) ) m_easingFunction = easeOutSine;
    // else if ( drawRadio( "Expo", E_TimeEasingType::E_Expo, true ) ) m_easingFunction = easeOutExpo;
    // else if ( drawRadio( "Bounce", E_TimeEasingType::E_Bounce, true ) ) m_easingFunction = easeOutBounce;
    // else if ( drawRadio( "Back", E_TimeEasingType::E_Back, true ) ) m_easingFunction = easeOutBack;
    // else if ( drawRadio( "Impulse", E_TimeEasingType::E_Impulse, true ) ) m_easingFunction = impulse;
    //
    // else if ( drawRadio( "PulseSine", E_TimeEasingType::E_PulseSine ) ) m_easingFunction = pulseSine;
    // else if ( drawRadio( "PulsePing", E_TimeEasingType::E_PulsePing, true ) ) m_easingFunction = pulsePing;
    // else if ( drawRadio( "SparkleFlicker", E_TimeEasingType::E_SparkleFlicker, true ) ) m_easingFunction = sparkleFlicker;
    // else if ( drawRadio( "SmoothPulse", E_TimeEasingType::E_SmoothPulseDecay, true ) ) m_easingFunction = smoothDecayPulse;

    ImGui::PopID();
  }

  // bool TimeEasing::drawRadio( const std::string& label,
  //                 const E_TimeEasingType easingType,
  //                 const bool useSameLine )
  // {
  //   if ( useSameLine ) ImGui::SameLine();
  //
  //   if ( ImGui::RadioButton( label.c_str(), easingType == m_data.easingType ) )
  //   {
  //     m_data.easingType = easingType;
  //     return true;
  //   }
  //
  //   return false;
  // }

  void TimeEasing::setEasingFunction( const E_TimeEasingType easingType )
  {
    switch ( easingType )
    {
      case E_TimeEasingType::E_Disabled:
      case E_TimeEasingType::E_Fixed:
      case E_TimeEasingType::E_TimeContinuous:
      case E_TimeEasingType::E_TimeIntervallic: m_easingFunction = useNone; break;

      case E_TimeEasingType::E_Quadratic: m_easingFunction = easeOutQuad; break;
      case E_TimeEasingType::E_Cubic: m_easingFunction = easeOutCubic; break;
      case E_TimeEasingType::E_Quartic: m_easingFunction = easeOutQuart; break;
      case E_TimeEasingType::E_Sine: m_easingFunction = easeOutSine; break;
      case E_TimeEasingType::E_Expo: m_easingFunction = easeOutExpo; break;
      case E_TimeEasingType::E_Bounce: m_easingFunction = easeOutBounce; break;
      case E_TimeEasingType::E_Back: m_easingFunction = easeOutBack; break;
      case E_TimeEasingType::E_PulsePing: m_easingFunction = pulsePing; break;
      case E_TimeEasingType::E_PulseSine: m_easingFunction = pulseSine; break;
      case E_TimeEasingType::E_Impulse: m_easingFunction = impulse; break;
      case E_TimeEasingType::E_SparkleFlicker: m_easingFunction = sparkleFlicker; break;
      case E_TimeEasingType::E_SmoothPulseDecay: m_easingFunction = smoothDecayPulse; break;
      case E_TimeEasingType::E_Elastic: m_easingFunction = easeOutElastic; break;
      default:
        m_easingFunction = easeOutLinear; break;
    }
  }

}