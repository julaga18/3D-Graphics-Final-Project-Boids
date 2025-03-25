#version 430 core

float AMBIENT = 0.3;

uniform vec3 color;
uniform sampler2D colorTexture;
uniform sampler2D normalSampler;
uniform float alpha;

uniform vec3 lightColor1;
uniform vec3 lightColor2;

uniform bool enableNormalMapping;

in vec2 vecTex;
in vec3 lightDirTS1;
in vec3 lightDirTS2;
in vec3 viewDirTS;
in mat3 TBN;

out vec4 outColor;

void main()
{
    vec3 N;
    if (enableNormalMapping) {
        // Normal mapping aktywne
        N = texture(normalSampler, vecTex).rgb;
        N = normalize(N * 2.0 - 1.0);  // Convert from [0, 1] to [-1, 1]
    } else {
        N = vec3(0.0, 0.0, 1.0);
    }

    vec3 L1 = normalize(lightDirTS1);
    vec3 L2 = normalize(lightDirTS2);
    vec3 V = normalize(viewDirTS);

    float diffuse1 = max(dot(N, L1), 0.0);
    float diffuse2 = max(dot(N, L2), 0.0);

    vec3 textureColor = texture(colorTexture, vecTex).rgb;

    // Combine lighting and color
    vec3 finalColor = (AMBIENT + diffuse1 * lightColor1 + diffuse2 * lightColor2) * textureColor;
    outColor = vec4(finalColor, alpha);
}