float4 sampleColor( float t ) {

  // Convert range 0..1 to 0..7
  float time_in_color_table = t * ( 8 - 1 );

  // if time_in_color_table = 1.2.      color_entry = 1., color_amount = 0.2
  float color_entry;
  float color_amount = modf( time_in_color_table, color_entry);
  float4 color_over_time = psystem_colors_over_time[ color_entry ] * ( 1 - color_amount )
                         + psystem_colors_over_time[ color_entry + 1 ] * ( color_amount );

  return color_over_time;
}

float sampleScale( float t ) {

  // Convert range 0..1 to 0..7
  float time_in_table = t * ( 8 - 1 );

  // if time_in_table = 1.2.      entry = 1., amount = 0.2
  float entry;
  float amount = modf( time_in_table, entry);
  float4 over_time = psystem_sizes_over_time[ entry ] * ( 1 - amount )
                   + psystem_sizes_over_time[ entry + 1 ] * ( amount );

  return over_time;
}
