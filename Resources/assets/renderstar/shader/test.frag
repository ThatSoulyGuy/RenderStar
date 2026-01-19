#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 tintColor;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 finalColor = mix(fragColor, tintColor.rgb, tintColor.a);
    outColor = vec4(finalColor, 1.0);
}
