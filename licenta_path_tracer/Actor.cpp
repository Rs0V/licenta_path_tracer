#include "Actor.hpp"
#include "glm.hpp"
#include "gtx/transform.hpp"


Actor::Actor()
	:
	transform(Transform()),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f),
	up(0.0f, 1.0f, 0.0f),
	components()
{
	auto rot = this->transform.rotation;
	this->transform.rotation = glm::vec3(0.0f);
	this->rotate(rot, 0);
}

Actor::Actor(Transform &&transform)
	:
	transform(std::move(transform)),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f),
	up(0.0f, 1.0f, 0.0f),
	components()
{
	auto rot = this->transform.rotation;
	this->transform.rotation = glm::vec3(0.0f);
	this->rotate(rot, 0);
}

void Actor::translate(glm::vec3 value, int type) {
	glm::vec3 xdir = type < 2 ? glm::vec3(1.0f, 0.0f, 0.0f) : this->right;
	glm::vec3 ydir = type < 2 ? glm::vec3(0.0f, 1.0f, 0.0f) : this->up;
	glm::vec3 zdir = type < 2 ? glm::vec3(0.0f, 0.0f, 1.0f) : this->forward;

	glm::vec3 trans = type < 1 ? value - this->transform.location : value;

	this->transform.location += xdir * trans.x + ydir * trans.y + zdir * trans.z;
}

void Actor::rotate(glm::vec3 value, int type) {
	glm::vec3 xdir = type < 2 ? glm::vec3(1.0f, 0.0f, 0.0f) : this->right;
	glm::vec3 ydir = type < 2 ? glm::vec3(0.0f, 1.0f, 0.0f) : this->up;
	glm::vec3 zdir = type < 2 ? glm::vec3(0.0f, 0.0f, 1.0f) : this->forward;

	glm::vec3 rot = type < 1 ? value - this->transform.rotation : value;
	rot = glm::radians(rot);

	float xrot = xdir.x * rot.x + ydir.x * rot.y + zdir.x * rot.z;
	float yrot = xdir.y * rot.x + ydir.y * rot.y + zdir.y * rot.z;
	float zrot = xdir.z * rot.x + ydir.z * rot.y + zdir.z * rot.z;


	auto rotation = glm::rotate(glm::mat4(1.0f), xrot, glm::vec3(1.0f, 0.0f, 0.0f));
	this->right   = glm::vec4(this->right, 1.0f) * rotation;
	this->forward = glm::vec4(this->forward, 1.0f) * rotation;
	this->up      = glm::vec4(this->up, 1.0f) * rotation;

	rotation = glm::rotate(glm::mat4(1.0f), yrot, glm::vec3(0.0f, 1.0f, 0.0f));
	this->right = glm::vec4(this->right, 1.0f) * rotation;
	this->forward = glm::vec4(this->forward, 1.0f) * rotation;
	this->up = glm::vec4(this->up, 1.0f) * rotation;

	rotation = glm::rotate(glm::mat4(1.0f), zrot, glm::vec3(0.0f, 0.0f, 1.0f));
	this->right = glm::vec4(this->right, 1.0f) * rotation;
	this->forward = glm::vec4(this->forward, 1.0f) * rotation;
	this->up = glm::vec4(this->up, 1.0f) * rotation;


	this->transform.rotation = glm::vec3(xrot, yrot, zrot);
	this->transform.rotation = glm::degrees(this->transform.rotation);
}

void Actor::scale(glm::vec3 value, int type) {
	glm::vec3 xdir = type < 2 ? glm::vec3(1.0f, 0.0f, 0.0f) : this->right;
	glm::vec3 ydir = type < 2 ? glm::vec3(0.0f, 1.0f, 0.0f) : this->up;
	glm::vec3 zdir = type < 2 ? glm::vec3(0.0f, 0.0f, 1.0f) : this->forward;

	glm::vec3 scl = type < 1 ? value / this->transform.scale : value;

	auto scaling = glm::scale(glm::mat4(1.0f), xdir * scl.x);
	this->transform.scale = glm::vec4(this->transform.scale, 1.0f) * scaling;

	scaling = glm::scale(glm::mat4(1.0f), ydir * scl.y);
	this->transform.scale = glm::vec4(this->transform.scale, 1.0f) * scaling;

	scaling = glm::scale(glm::mat4(1.0f), zdir * scl.z);
	this->transform.scale = glm::vec4(this->transform.scale, 1.0f) * scaling;
}
