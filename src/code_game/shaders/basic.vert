#version 460 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 1) in vec3 aColor; 
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTextureCoordinates;

out vec3 vColor;
out vec2 vTextureCoordinates;
out vec3 vNormalVS;
out vec3 vNormalWS;
out vec3 vSunlightDirVS;
out vec3 vPosVS;
out vec3 vPosWS;
out vec4 vCoordLightSpace;
out vec3 vVdirTangentSpaceTracks;
out vec3 vLdirTangentSpaceTracks;
out vec4 vFanaliTextureCoord[10];
out vec4 vLampCoordLightSpace[10];
out vec4 vSkyboxTexCoord;





uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform int uObject;
uniform mat4 uLightMatrix;

uniform mat4 uFanaliView[10]; 
uniform mat4 uFanaliProj; 

uniform mat4 uLampLightMatrix[10];
uniform vec3 uSunlightDir;
uniform vec3 uLampPointLight[10]; 
uniform vec3 uLampLightTarget[10]; 
//uniform vec4 uLampLightDirection[10];

uniform int uLampAttivo[10]; 

void main(void) 
{ 
	
	vColor = aColor;
	vTextureCoordinates = aTextureCoordinates;

	vSunlightDirVS = (uView*vec4(uSunlightDir,0.0)).xyz;
	
	
	//vPosition in World space la uso per calcolare il lighting dei lampioni;
	
	vPosWS = (uModel*vec4(aPosition,1.0)).xyz;
	vNormalWS = (uModel*vec4(aNormal, 0.0)).xyz;
	
	vNormalVS = (uView*vec4(vNormalWS, 0.0)).xyz;
	vPosVS = (vec4(0.0,0.0,0.0,1.0) -uView*uModel*vec4(aPosition, 1.0)).xyz;
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 

	
	for(int i = 0; i < 10; i++){
		
		

		vLampCoordLightSpace[i] = uLampLightMatrix[i] * uModel * vec4(aPosition,1.0);

		
	}

	vCoordLightSpace = uLightMatrix * uModel * vec4(aPosition,1.0);
	vSkyboxTexCoord =  inverse(uView)*(uView*uModel*vec4(aPosition, 1.0)-vec4(0,0,0,1.0));

	//tangent frame per tracks
	if(uObject == 0){
		for(int i = 0; i < 10; i++){
			vFanaliTextureCoord[i] = uFanaliProj * uFanaliView[i] * uModel * vec4(aPosition,1.0);
		}
		// computing the (inverse of the ) tangent frame
		vec3 tangent = normalize(aTangent);
		vec3 bitangent = normalize(cross(aNormal,tangent));
	
		mat3 TF;
		TF[0]	= tangent;
		TF[1]	= bitangent;
		TF[2]	= normalize(aNormal);
		TF		= transpose(TF);

		// light direction in tangent space
		vLdirTangentSpaceTracks   =    TF * (inverse(uModel)*vec4(uSunlightDir,0.0)).xyz;

		vec3 ViewVS  =  (vec4(0.0,0.0,0.0,1.0) -uView*uModel*vec4(aPosition, 1.0)).xyz; 

		// view direction in tangent space
		vVdirTangentSpaceTracks	  =    TF * (inverse(uModel)*inverse(uView)* vec4(ViewVS,0.0)).xyz;
	}
}