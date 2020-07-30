#version 330 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D texture_sample;
uniform vec4 objColor;

void main() {
    fragColor = texture(texture_sample, texCoord) * objColor;
}
