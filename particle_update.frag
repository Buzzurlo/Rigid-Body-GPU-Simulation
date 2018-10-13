#version 430

uniform sampler2D posTexture;
uniform sampler2D velTexture;
uniform int texSize;
uniform float dt;

layout (location = 0) out vec4 pos;
layout (location = 1) out vec4 vel;

void CollideSphereDiscrete(in vec4 thisPos, inout vec4 thisVel, in vec4 thatPos, in vec4 thatVel, float dt) {
	float r = 1;
	float r2 = (r + r)*(r + r);
	
	vec3 dir = thatPos.xyz - thisPos.xyz;

	float dist2 = dot(dir, dir);

	if (dist2 < r2) {
		dir = normalize(dir);
		float a1 = dot(thisVel.xyz, dir);
		float a2 = dot(thatVel.xyz, dir);

		//thisVel.xyz = thisVel.xyz - 0.1f * (thisVel.xyz - a1);

		thisVel.xyz = thisVel.xyz - (a1 - a2) * dir;
		//thatVel.xyz = thatVel.xyz + (a1 - a2) * dir;
	}
}

vec3 CollideSphereContinous(inout vec4 thisPos, inout vec4 thisVel, in vec4 thatPos, in vec4 thatVel, float dt) {
	float r = 0.25;
	float r2 = (r + r) * (r + r);
	vec3 dir = thatPos.xyz - thisPos.xyz;
	vec3 v = thatVel.xyz - thisVel.xyz;
	vec3 p = thatPos.xyz - thisPos.xyz;
	float a = dot(v, v);
	float b = dot(v, p);
	float c = dot(p, p) - r2;
	float d = b*b - a*c;

	float t;

	if (c < 0) {
		// already overlapping
		float x = r + r - length(p);
		float k = 200;
		float d = 0.7f;
		float f = 0.3f;

		dir = normalize(dir);
		vec3 fs = -k * x * dir;
		vec3 fd = (d - 1) * thisVel.xyz;
		vec3 v_t = thisVel.xyz - dot(thisVel.xyz, dir) * dir;
		vec3 ft = -f * v_t;
		//thisVel.xyz += (fs + fd + ft) * dt;

		return (fs + fd + ft);
	}
	
	
	if (b <= 0 && d >= 0) {
		// they are moving towards each other
		// if delta has real roots we have collision
		t = (-b - sqrt(d)) / a;
		if (t <= dt) {
			// only interested if it happened in this frame
			vec3 pos1 = thisVel.xyz * t + thisPos.xyz;
			vec3 pos2 = thatVel.xyz * t + thatPos.xyz;
			dir = normalize(pos2 - pos1);
			float a1 = dot(thisVel.xyz, dir);
			float a2 = dot(thatVel.xyz, dir);

			vec3 old = thisVel.xyz;
			//thisVel.xyz = thisVel.xyz - 0.01f * (thisVel.xyz - a1);
			thisVel.xyz = thisVel.xyz - (a1 - a2) * dir;

			//thisPos.xyz = thisVel.xyz * (dt - t) + pos1;
		}
	}
	return vec3(0);
}
void BoundsCheck(inout vec4 pos, inout vec4 vel, in float limit) {
	if (pos.y < 0.25) {
		pos.y = 0.25;
		vel.y *= -1;
	} else if (pos.y > limit) {
		pos.y = limit;
		vel.y *= -1;
	}
	if (pos.x < -limit) {
		pos.x = -limit;
		vel.x *= -1;
	}
	else if (pos.x > limit) {
		pos.x = limit;
		vel.x *= -1;
	}
	if (pos.z < -limit) {
		pos.z = -limit;
		vel.z *= -1;
	}
	else if (pos.z > limit) {
		pos.z = limit;
		vel.z *= -1;
	}
}

void main() {
	vec2 uv = gl_FragCoord.xy / texSize;
	pos = texture(posTexture, uv);
	vel = texture(velTexture, uv);

	// collision
	int i;
	vec4 force = vec4(0);
	for(i = 0; i < texSize*texSize; i = i+1) {
		vec2 otherUV = vec2(i % texSize, i / texSize) / texSize + 1.0f / (2*texSize);
		force.xyz += CollideSphereContinous(pos, vel, texture(posTexture, otherUV), texture(velTexture, otherUV), dt);
	}

	// integration
	vec4 acc = vec4(0.0f, -9.81f, 0.0f, 0.0f);
	//vel = 0.999f * vel;

	force += acc;
	vel = vel + force * dt;
	pos = pos + vel * dt;

	// bounds check
	int ball_radius = 1;
	int box_half_side = 10;
	BoundsCheck(pos, vel, 12.0f);

	pos.w = 1.0f;
	vel.w = 1.0f;
}