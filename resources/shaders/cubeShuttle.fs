#version 330 core
out vec4 FragColor;


in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuse;
uniform int i;
uniform vec3 viewPosition;


void main()
{
    vec4 texColor=texture(diffuse,TexCoords);
    if(i==0){
        texColor.a=0.1;
    }
    FragColor = texColor;
}