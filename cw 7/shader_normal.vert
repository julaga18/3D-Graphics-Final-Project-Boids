#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 cameraPos;

out vec2 vecTex;
out vec3 lightDirTS1;
out vec3 lightDirTS2;
out vec3 viewDirTS;
out mat3 TBN;

void main()
{
    // Transform vectors to world space
    vec3 worldPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
    vec3 normal = normalize((modelMatrix * vec4(vertexNormal, 0.0)).xyz);
    vec3 tangent = normalize((modelMatrix * vec4(vertexTangent, 0.0)).xyz);
    vec3 bitangent = normalize((modelMatrix * vec4(vertexBitangent, 0.0)).xyz);

    TBN = transpose(mat3(tangent, bitangent, normal));

    // Compute light and view directions in world space
    vec3 lightDir1 = lightPos1 - worldPos;
    vec3 lightDir2 = lightPos2 - worldPos;
    vec3 viewDir = cameraPos - worldPos;

    // Transform to tangent space
    lightDirTS1 = TBN * lightDir1;
    lightDirTS2 = TBN * lightDir2;
    viewDirTS = TBN * viewDir;

    vecTex = vec2(vertexTexCoord.x, 1.0 - vertexTexCoord.y);
    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
