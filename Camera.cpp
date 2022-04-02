#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //Update the rest of camera parameters
		this->cameraFrontDirection = glm::normalize(-cameraPosition + cameraTarget);
		this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
		this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
		if (direction == MOVE_FORWARD) {
			cameraPosition += cameraFrontDirection * speed;
			cameraTarget += cameraFrontDirection * speed;
		}

		if (direction == MOVE_BACKWARD) {
			cameraPosition -= cameraFrontDirection * speed;
			cameraTarget -= cameraFrontDirection * speed;
		}

		if (direction == MOVE_LEFT) {
			cameraPosition -= cameraRightDirection * speed;
			cameraTarget -= cameraRightDirection * speed;
		}

		if (direction == MOVE_RIGHT) {
			cameraPosition += cameraRightDirection * speed;
			cameraTarget += cameraRightDirection * speed;
		}

		if (direction == MOVE_UP) {
			cameraPosition += cameraUpDirection * speed;
			cameraTarget += cameraUpDirection * speed;
		}

		if (direction == MOVE_DOWN) {
			cameraPosition -= cameraUpDirection * speed;
			cameraTarget -= cameraUpDirection * speed;
		}

		if (direction == ROTATE_RIGHT) {
			cameraTarget.x -= speed;
		}

		if (direction == ROTATE_LEFT) {
			cameraTarget.x += speed;
		}

		if (direction == ROTATE_UP) {
			cameraTarget.x -= speed;
		}

		if (direction == ROTATE_DOWN) {
			cameraTarget.x += speed;
		}
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
		this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
		cameraTarget = cameraFrontDirection + cameraPosition;

		//printf("fornt.x = %f, front.y = %f, front.z = %f\n", front.x, front.y, front.z);
    }
}