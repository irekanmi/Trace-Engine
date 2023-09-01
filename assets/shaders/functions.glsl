
vec4 prefilter_color(vec4 color, float threshold)
{
    float brightness = max(max(color.r, color.g), color.b);
    float numerator = max(brightness - threshold, 0.0001f);
    float denominator = max(brightness, 0.0001f);
    float result = numerator / denominator;
    return color * result;
}