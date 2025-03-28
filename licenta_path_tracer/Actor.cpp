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

inline glm::mat2 rot2D(float angle) {
	float s = glm::sin(angle);
	float c = glm::cos(angle);
	return glm::mat2(c, -s, s, c);
}

void Actor::rotate(glm::vec3 value, int type, std::optional<glm::vec3> pivot) {
	defaults(pivot, glm::vec3(0.0f, 0.0f, 0.0f));

	glm::vec3 xdir = type < 2 ? glm::vec3(1.0f, 0.0f, 0.0f) : this->right;
	glm::vec3 ydir = type < 2 ? glm::vec3(0.0f, 1.0f, 0.0f) : this->up;
	glm::vec3 zdir = type < 2 ? glm::vec3(0.0f, 0.0f, 1.0f) : this->forward;

	glm::vec3 rot = type < 1 ? value - this->transform.rotation : value;
	rot = glm::radians(rot);

	float xrot = xdir.x * rot.x + ydir.x * rot.y + zdir.x * rot.z;
	float yrot = xdir.y * rot.x + ydir.y * rot.y + zdir.y * rot.z;
	float zrot = xdir.z * rot.x + ydir.z * rot.y + zdir.z * rot.z;


	auto calcDir = [&](glm::vec3 &dir) {
		glm::vec2 yz = glm::vec2(dir.y, dir.z) * rot2D(xrot);
		dir = { dir.x, yz[0], yz[1] };
		glm::vec2 xz = glm::vec2(dir.x, dir.z) * rot2D(yrot);
		dir = { xz[0], dir.y, xz[1] };
		glm::vec2 xy = glm::vec2(dir.x, dir.y) * rot2D(zrot);
		dir = { xy[0], xy[1], dir.z };

		dir = glm::normalize(dir);
		if (glm::abs(dir.x) < 0.0001) {
			dir.x = 0.0f;
		}
		if (glm::abs(dir.y) < 0.0001) {
			dir.y = 0.0f;
		}
		if (glm::abs(dir.z) < 0.0001) {
			dir.z = 0.0f;
		}
	};
	calcDir(this->right);
	calcDir(this->forward);
	calcDir(this->up);

	this->transform.rotation += glm::degrees(glm::vec3(xrot, yrot, zrot));


	glm::mat4 dpMat  = glm::translate(glm::mat4(1.0f), -pivot.value() - this->transform.location);
	glm::mat4 idpMat = glm::translate(glm::mat4(1.0f),  pivot.value() + this->transform.location);

	glm::mat4 rotMat = glm::mat4(1.0f)
		* glm::rotate(glm::mat4(1.0f), xrot, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), yrot, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), zrot, glm::vec3(0.0f, 0.0f, 1.0f))
	;
	this->transform.location = glm::vec3(idpMat * rotMat * dpMat * glm::vec4(this->transform.location, 1.0f));
}

void Actor::scale(glm::vec3 value, int type, std::optional<glm::vec3> pivot) {
	defaults(pivot, glm::vec3(0.0f, 0.0f, 0.0f));

	glm::vec3 xdir = type < 2 ? glm::vec3(1.0f, 0.0f, 0.0f) : this->right;
	glm::vec3 ydir = type < 2 ? glm::vec3(0.0f, 1.0f, 0.0f) : this->up;
	glm::vec3 zdir = type < 2 ? glm::vec3(0.0f, 0.0f, 1.0f) : this->forward;

	glm::vec3 scl = type < 1 ? value / this->transform.scale : value;

	float xscl = xdir.x * scl.x + ydir.x * scl.y + zdir.x * scl.z;
	float yscl = xdir.y * scl.x + ydir.y * scl.y + zdir.y * scl.z;
	float zscl = xdir.z * scl.x + ydir.z * scl.y + zdir.z * scl.z;


	glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(xscl, yscl, zscl));
	this->transform.scale = glm::vec3(scaling * glm::vec4(this->transform.scale, 1.0f));


	glm::mat4 dpMat  = glm::translate(glm::mat4(1.0f), -pivot.value() - this->transform.location);
	glm::mat4 idpMat = glm::translate(glm::mat4(1.0f),  pivot.value() + this->transform.location);

	this->transform.location = glm::vec3(idpMat * scaling * dpMat * glm::vec4(this->transform.location, 1.0f));
}
