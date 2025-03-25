#version 430 core
in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D backgroundTexture;

void main() {
    outColor = texture(backgroundTexture, fragTexCoord);
}