Shader "Custom/ChromaKey" {
    Properties {
        _MainTex ("Base (RGB)", 2D) = "white" {}
        _TransparentColourKey ("Transparent Colour Key", Color) = (0,0,0,1)
        _TransparencyTolerance ("Transparency Tolerance", Float) = 0.01
        _SwapRedBlue ("Swap Red and Blue Chanels", Int) = 0
        _PreMultiplyAlpha ("PreMultiply Alpha Channels", Int) = 0
    }
    SubShader {
        Pass {
            Tags { "RenderType" = "Opaque" }
            LOD 200
         
            CGPROGRAM
 
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"
 
            struct a2v
            {
                float4 pos : POSITION;
                float2 uv : TEXCOORD0;
            };
 
            struct v2f
            {
                float4 pos : SV_POSITION;
                float2 uv : TEXCOORD0;
            };
 
            v2f vert(a2v input)
            {
                v2f output;
                output.pos = mul (UNITY_MATRIX_MVP, input.pos);
                output.uv = input.uv;
                return output;
            }
         
            sampler2D _MainTex;
            float3 _TransparentColourKey;
            float _TransparencyTolerance;
            int _SwapRedBlue;
            int _PreMultiplyAlpha;
 
            float4 frag(v2f input) : SV_Target
            {
                // What is the colour that *would* be rendered here?
                float4 colour = tex2D(_MainTex, input.uv);
             
                // Calculate the different in each component from the chosen transparency colour
                float deltaR = abs(colour.r - _TransparentColourKey.r);
                float deltaG = abs(colour.g - _TransparentColourKey.g);
                float deltaB = abs(colour.b - _TransparentColourKey.b);

                if (_SwapRedBlue)
                {
                	float temp = colour.r;
                	colour.r = colour.b;
                	colour.b = temp;
                }

                if (_PreMultiplyAlpha)
                {
                	colour.rgb *= colour.a;
                }
 
                // If colour is within tolerance, write a transparent pixel
                //if (deltaR < _TransparencyTolerance && deltaG < _TransparencyTolerance && deltaB < _TransparencyTolerance)
                //{
                //    return float4(0.0f, 0.0f, 0.0f, 0.0f);
                //}
 
                // Otherwise, return the regular colour
                return colour;
            }
            ENDCG
        }
    }
}