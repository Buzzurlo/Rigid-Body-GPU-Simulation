#include "Particle.h"

ParticleManager::ParticleManager(float diam)
{
	sphere.add_sphere(vec3(0, 0, 0), diam / 2);
	sphere.save_changes();

	shader.attach(GL_VERTEX_SHADER, "instanced.vert");
	shader.attach(GL_FRAGMENT_SHADER, "gpu_shader.frag");
	shader.link();
}

Mesh3 ParticleManager::get_model()
{
	return sphere;
}

Shader& ParticleManager::get_shader()
{
	return shader;
}

int ParticleManager::get_count()
{
	return particles.size();
}

GLuint ParticleManager::buildBuffer()
{
	GLuint dataBuffer;
	glGenBuffers(1, &dataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuffer);
	// use this later, for variable body count
	// glBufferData(GL_ARRAY_BUFFER, sizeof(RigidBodyInfo) * MAX_BODIES, NULL, GL_DYNAMIC_COPY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleInfo) * particles.size(), particles.data(), GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dataBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	return dataBuffer;
}

int ParticleManager::add_particles(std::vector<ParticleInfo> data)
{
	int offset = particles.size();
	particles.insert(particles.end(), data.begin(), data.end());

	return offset;
}

void ParticleManager::load_particles()
{
	sphere.bind();

	// probably very bad when having tons of particles
	int n = particles.size();
	vec3 * particles_positions = new vec3[n];
	for (int i = 0; i < n; ++i) 
		particles_positions[i] = particles[i].currentPosition;

	sphere.very_bad_instanced_data_setup(particles_positions, n * sizeof(vec3));
	glFinish();
	delete particles_positions;
}
