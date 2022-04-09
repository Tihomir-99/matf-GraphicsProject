#version 330 core
out vec4 FragColor;
struct Material {
    sampler2D texture_diffuse1;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform Material material;


void main()
{
    FragColor = texture(material.texture_diffuse1,TexCoords);
}