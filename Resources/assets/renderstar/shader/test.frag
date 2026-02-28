#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 tintColor;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    vec3 baseColor = fragColor * texColor.rgb;
    vec3 finalColor = mix(baseColor, tintColor.rgb, tintColor.a);
    outColor = vec4(finalColor, 1.0);
}
