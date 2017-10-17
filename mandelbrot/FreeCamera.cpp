#include "FreeCamera.h"

FreeCamera::FreeCamera() {
	position.set(0.0f, 0.0f, 2.4f);
	forward.set(0.0f, 0.0f, 0.0f);
	up.set(0.0f, 0.0f, 0.0f);
	side.set(0.0f, 0.0f, 0.0f);
	lookAt.set(1.0f, 1.0f, 5.0f);
	setYaw(0);
	setPitch(0);
	setRoll(0);
	update();
}

FreeCamera::~FreeCamera() {}

void FreeCamera::update() {
	float cosR, cosP, cosY;	//temp values for sin/cos from 
	float sinR, sinP, sinY;
	// Roll, Pitch and Yall are variables stored by the FreeCamera
	// handle rotation
	// Only want to calculate these values once, when rotation changes, not every frame. 
	cosY = cosf( (Yaw*3.1415f) / 180.0f);
	cosP = cosf( (Pitch*3.1415f) / 180.0f);
	cosR = cosf( (Roll*3.1415f) / 180.0f);
	sinY = sinf( (Yaw*3.1415f) / 180.0f);
	sinP = sinf( (Pitch*3.1415f) / 180.0f);
	sinR = sinf( (Roll*3.1415f) / 180.0f);
	// Calculate forward vector
	forward.x = sinY * cosP;
	forward.y = sinP;
	forward.z = cosP * -cosY;
	// Calculate lookAt vector
	lookAt.x = position.x + forward.x;
	lookAt.y = position.y + forward.y;
	lookAt.z = position.z + forward.z;
	// Calculate up Vector
	up.x = -cosY * sinR - sinY * sinP * cosR;
	up.y = cosP * cosR;
	up.z = -sinY * sinR - sinP * cosR * -cosY;
	// Calculate side Vector (right)
	side = forward.cross(up); // this is a cross product between the forward and up vector
}

float FreeCamera::getPositionX() {
	return position.getX();
}
float FreeCamera::getPositionY() {
	return position.getY();
}
float FreeCamera::getPositionZ() {
	return position.getZ();
}

float FreeCamera::getForwardX() {
	return forward.getX();
}
float FreeCamera::getForwardY() {
	return forward.getY();
}
float FreeCamera::getForwardZ() {
	return forward.getZ();
}

float FreeCamera::getLookAtX() {
	return lookAt.getX();
}
float FreeCamera::getLookAtY() {
	return lookAt.getY();
}
float FreeCamera::getLookAtZ() {
	return lookAt.getZ();
}

float FreeCamera::getUpX() {
	return up.getX();
}
float FreeCamera::getUpY() {
	return up.getY();
}
float FreeCamera::getUpZ() {
	return up.getZ();
}

float FreeCamera::getSideX() {
	return side.getZ();
}
float FreeCamera::getSideY() {
	return side.getY();
}
float FreeCamera::getSideZ() {
	return side.getX();
}

float FreeCamera::getYaw() {
	return Yaw;
}
float FreeCamera::getPitch() {
	return Pitch;
}
float FreeCamera::getRoll() {
	return Roll;
}

void FreeCamera::setLookAtX(float x) {
	lookAt.setX(x);
}
void FreeCamera::setLookAtY(float y) {
	lookAt.setY(y);
}
void FreeCamera::setLookAtZ(float z) {
	lookAt.setZ(z);
}

void FreeCamera::setYaw(float arg) {
	Yaw = arg;
}
void FreeCamera::setPitch(float arg) {
	Pitch = arg;
}
void FreeCamera::setRoll(float arg) {
	Roll = arg;
}

void FreeCamera::moveForward(float dt) {
	position.add(forward, dt);
}
void FreeCamera::moveBackwards(float dt) {
	position.subtract(forward, dt);
}

void FreeCamera::moveUp(float dt) {
	position.add(up, dt);
}
void FreeCamera::moveDown(float dt) {
	position.subtract(up, dt);
}

void FreeCamera::moveSideLeft(float dt) {
	position.subtract(side, dt);
}

void FreeCamera::moveSideRight(float dt) {
	position.add(side, dt);
}

void FreeCamera::addYaw(float dt, float value) {
	Yaw += value * dt;
}
void FreeCamera::subtractYaw(float dt, float value) {
	Yaw -= value * dt;
}
void FreeCamera::addPitch(float dt, float value) {
	Pitch += value * dt;
}
void FreeCamera::subtractPitch(float dt, float value) {
	Pitch -= value * dt;
}
void FreeCamera::addRoll(float dt, float value) {
	Roll += value * dt;
}
void FreeCamera::subtractRoll(float dt, float value) {
	Roll -= value * dt;
}

void FreeCamera::updateYaw(int width, int mouseX, int speed) {
	Yaw += static_cast<float>((mouseX - (width / 2)) / speed);
}
void FreeCamera::updatePitch(int height, int mouseY, int speed) {
	Pitch -= static_cast<float>((mouseY - (height / 2)) / speed);
}

void FreeCamera::cameraControll(float dt, int width, int height, Input *input) {
	// move camera left
	if (input->isKeyDown('a') || input->isKeyDown('A')) {
		moveSideLeft(dt);
	}
	// move camera right
	if (input->isKeyDown('d') || input->isKeyDown('D')) {
		moveSideRight(dt);
	}
	// move camera up
	if (input->isKeyDown('w') || input->isKeyDown('W')) {
		moveUp(dt);
	}
	// move camera down
	if (input->isKeyDown('s') || input->isKeyDown('S')) {
		moveDown(dt);
	}
}
