#version 460 core  
out vec4 color; 

in vec3 vColor; 
in vec2 vTextureCoordinates;
in vec3 vSunlightDirVS;
in vec3 vPosVS;
in vec3 vPosWS;
in vec3 vNormalVS;
in vec3 vNormalWS;
in vec3 vVdirTangentSpaceTracks;
in vec3 vLdirTangentSpaceTracks;
in vec4 vCoordLightSpace;
in vec4 vFanaliTextureCoord[10];
in vec4 vLampCoordLightSpace[10];
in vec4 vSkyboxTexCoord;


uniform int uObject;
uniform vec3 uColor;
uniform sampler2D uModelTexture;
uniform sampler2D uTrackTexture;
uniform sampler2D uTerrainTexture;
uniform sampler2D uNormalMapTracks;
uniform sampler2D uFanaliTexture; //proviamo tu 7;
uniform sampler2D uShadowMap;
uniform ivec2 uShadowMapSize;
uniform float uBias;
uniform samplerCube uSkybox;

//roba lighting lampioni;

uniform vec3 uLampPointLight[10]; 
uniform vec3 uLampLightTarget[10]; 
uniform float uThetaIn;
uniform float uThetaOut;
uniform float uBlurLamp;
uniform float uRaggioLampioni;
uniform float uLampBias;

//uniform sampler2DArray uLampShadowMaps;

uniform sampler2D uLampioniShadowMaps[10]; 
uniform int uLampAttivo[10];




uniform int uApprossimaNormaliTerreno;
uniform vec3 uLightColor;

float linstep(float low, float high, float v){
    return clamp((v-low)/(high-low), 0.0, 1.0);
}



vec3 lamp_lighting(vec3 lightPoint, vec3 lightTarget,vec3 N, vec3 diffColor, int n){

    
    if( uLampAttivo[n]==1){
        
        vec4 shadowCoord = (vLampCoordLightSpace[n]/vLampCoordLightSpace[n].w)*0.5+0.5;
        //vec4 shadowCoord = (vLampCoordLightSpace[n])*0.5+0.5;
        float depth = texture(uLampioniShadowMaps[n],shadowCoord.xy).x;
        
        if(depth + uLampBias < shadowCoord.z ){
            return vec3(0.f,0.f,0.f);
        }
            
    }
   
       

    vec3 vPosLPWS = vPosWS - lightPoint;
    if(dot(vPosLPWS,vPosLPWS) > uRaggioLampioni) return vec3(0.f,0.f,0.f); 
	
    vec3 lightDirection = lightTarget - lightPoint;
	
	float alpha = (dot( normalize(lightDirection), normalize(vPosLPWS) ) ) ;
    
    float a = acos(alpha); 

    float LN = max( dot( -normalize(vPosLPWS),normalize(N) ), 0.0);
    
	if(a < uThetaIn) return (LN * diffColor) * vec3(0.8f,0.8f,0.8f);
	else if(a > uThetaOut) return vec3(0.f,0.f,0.f);
	else return (LN * diffColor) * ( vec3(0.7f,0.7f,0.7f) * pow(cos(alpha-uThetaIn),uBlurLamp) );
}

vec3 phong(vec3 L, vec3 N, vec3 V, vec3 diffColor, vec3 specColor, float shininess){
    
    float bias = clamp(uBias*tan(acos(dot(N,L))),uBias,0.05);
    float lit = 1.f;
    vec4 pLS = (vCoordLightSpace/vCoordLightSpace.w)*0.5+0.5;
    float depth = texture(uShadowMap,pLS.xy).x;
	if(depth + bias < pLS.z ){
        lit = 0.0;
       
    }
    
    

    float LN = max(0.0,dot(L,N));

    vec3 R = -L + 2*(dot(N,L))*N;//nota che R serve solo per specComponent;
    //float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),shininess));

    if(L.y < 0) LN = 0;
    float spec = 0;
	if(LN > 0.f) spec = max( 0.0, pow(dot(R,V), shininess));
    
    return (diffColor * 0.15 + LN * diffColor * lit + spec * specColor * lit) * uLightColor;

}

/* 
vec2 m = texture( uShadowMap, pLS.xy).xy;
 	float mu = m.x;
	// do not trust 0 variance
	float sigma = max(m.y-mu*mu,0.001);
	float diff = pLS.z - mu; 
	if(diff > 0.0){
		lit = sigma / (sigma+diff*diff);

		//remap the formula between 0.001 and 1.0
		lit = linstep(0.001, 1.0, sigma / (sigma + diff*diff));
    }                
	*/


    

    

void main(void) 
{   
    color = vec4((uColor.x<0)?vColor:uColor, 1.0);
    color = texture(uSkybox,normalize(vSkyboxTexCoord.xyz));
    if(uObject == 0)//0 = track
    {
        vec3 N = vec3(texture2D(uNormalMapTracks,vTextureCoordinates));
		N = normalize(N*2.0-1.0);
		vec2 fanaliTextureCoord[10];
        vec4 fanaliColor[10];
        vec3 tex_col = vec3(texture2D(uTrackTexture,vTextureCoordinates));
        //color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS), normalize(vPosVS),tex_col,tex_col,2), 1.0);
        color = vec4(phong(normalize(vLdirTangentSpaceTracks), N ,normalize(vVdirTangentSpaceTracks),tex_col,tex_col,2),1.0);
        
        for(int i = 0; i < 10; i++){
            if(vFanaliTextureCoord[i].w <= 1.f ) {
                fanaliTextureCoord[i] = ((vFanaliTextureCoord[i] / vFanaliTextureCoord[i].w).xy *0.5 + 0.5);
                fanaliColor[i] = texture2D(uFanaliTexture, fanaliTextureCoord[i]);
                color += vec4(tex_col * (fanaliColor[i].xyz * fanaliColor[i].w),1.0);
               
            }
        }
        
        
        for(int j = 0; j < 10; j++){
            
            color += vec4(lamp_lighting(uLampPointLight[j],uLampLightTarget[j],vNormalWS, tex_col,j),1.0);
        
        }
       
        
        
    }
    else if(uObject == 1)//1 = terreno
    {   
        vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
        vec3 NWS = normalize( cross( dFdx(vPosWS), dFdy(vPosWS) ) );
        vec3 tex_col = texture2D(uTerrainTexture, vTextureCoordinates).xyz;
        //color = texture2D(uTerrainTexture, vTextureCoordinates);
        
        
        if(uApprossimaNormaliTerreno==0) color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS), normalize(vPosVS),tex_col,tex_col,2), 1.0);
        else color = vec4(phong(normalize(vSunlightDirVS),normalize(N),normalize(vPosVS),tex_col,tex_col,2),1.0);
        for(int i = 0; i < 10; i++){ 
            if(uApprossimaNormaliTerreno==0) color += vec4(lamp_lighting(uLampPointLight[i],uLampLightTarget[i],vNormalWS, tex_col,i),1.0);
            else color += vec4(lamp_lighting(uLampPointLight[i],uLampLightTarget[i],NWS, tex_col,i),1.0);
        }
    }
    else if(uObject == 3)//3 = taxi
    {
        vec3 tex_col = vec3(texture2D(uModelTexture,vTextureCoordinates));
        color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS),normalize(vPosVS),tex_col,tex_col,16),1.0);
        for(int i = 0; i < 10; ++i){
                color += vec4(lamp_lighting(uLampPointLight[i],uLampLightTarget[i],vNormalWS, tex_col,i),1.0);
        
        }
    }
    else if(uObject == 4)//4 = cameraman
    {
         vec3 tex_col = vec3(texture2D(uModelTexture,vTextureCoordinates));
        color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS),normalize(vPosVS),tex_col,tex_col,2),1.0);
    }
    else if(uObject == 5) // 5 = tree
    {
         vec3 tex_col = vec3(texture2D(uModelTexture,vTextureCoordinates));
        color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS),normalize(vPosVS),tex_col,tex_col,2),1.0);
    }
    else if(uObject == 6) //lampioni
    {
        vec3 tex_col = vec3(texture2D(uModelTexture,vTextureCoordinates));
        color = vec4(phong(normalize(vSunlightDirVS), normalize(vNormalVS),normalize(vPosVS),tex_col,tex_col,2),1.0);
        for(int j = 0; j < 10; j++){
            color += vec4(lamp_lighting(uLampPointLight[j],uLampLightTarget[j],vNormalWS, tex_col,j),1.0);
        }    
    }
    
} 