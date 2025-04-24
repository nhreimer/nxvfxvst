// #pragma once
//
// #include <bitset>
//
// namespace nx
// {
//   template < typename TShaderParam >
//   class EasingAssignmentComponent final
//   {
//     static_assert( std::is_enum_v< TShaderParam >, "T must be an enum type" );
//   public:
//     void drawMenu();
//
//     bool isAssigned( const TShaderParam param ) const;
//
//   private:
//     std::bitset<static_cast<size_t>(TShaderParam::LastItem)> m_assignments;
//   };
// }