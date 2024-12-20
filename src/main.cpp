/**
This application displays a mesh in wireframe using "Modern" OpenGL 3.0+.
The Mesh3D class now initializes a "vertex array" on the GPU to store the vertices
	and faces of the mesh. To render, the Mesh3D object simply triggers the GPU to draw
	the stored mesh data.
We now transform local space vertices to clip space using uniform matrices in the vertex shader.
	See "simple_perspective.vert" for a vertex shader that uses uniform model, view, and projection
		matrices to transform to clip space.
	See "uniform_color.frag" for a fragment shader that sets a pixel to a uniform parameter.
*/
#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <filesystem>
#include <math.h>

#include "AssimpImport.h"
#include "Mesh3D.h"
#include "Object3D.h"
#include "Animator.h"
#include "ShaderProgram.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Audio.hpp>

struct Scene {
	ShaderProgram program;
	std::vector<Object3D> objects;
	std::vector<Animator> animators;
};

/**
 * @brief Constructs a shader program that applies the Phong reflection model.
 */
ShaderProgram phongLightingShader() {
	ShaderProgram shader;
	try {
		// These shaders are INCOMPLETE.
		shader.load("shaders/light_perspective.vert", "shaders/lighting.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Constructs a shader program that performs texture mapping with no lighting.
 */
ShaderProgram texturingShader() {
	ShaderProgram shader;
	try {
		shader.load("shaders/texture_perspective.vert", "shaders/texturing.frag");
	}
	catch (std::runtime_error& e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
	return shader;
}

/**
 * @brief Loads an image from the given path into an OpenGL texture.
 */
Texture loadTexture(const std::filesystem::path& path, const std::string& samplerName = "baseTexture") {
	StbImage i;
	i.loadFromFile(path.string());
	return Texture::loadImage(i, samplerName);
}

Scene arcadeScene() {
	Scene scene{ phongLightingShader() };
	//Scene scene{ texturingShader() };

	// loading in arcade machines, 12 TOTAL
	auto pacman = assimpLoad("models/pacman/scene.gltf", true);
	auto finalFight = assimpLoad("models/finalFight/scene.gltf", true);
	auto streetFighter = assimpLoad("models/streetFighter/scene.gltf", true);
	auto pixelPoro = assimpLoad("models/pixelPoro/scene.gltf", true);
	auto kirby = assimpLoad("models/kirby/scene.gltf", true);
	auto cyberWing = assimpLoad("models/cyberSwing/scene.gltf", true);
	auto ddr = assimpLoad("models/ddr/scene.gltf", true);
	auto diablo = assimpLoad("models/diablo/scene.gltf", true);
	auto rune = assimpLoad("models/rune/scene.gltf", true);
	auto mortalKombat = assimpLoad("models/mortalKombat/scene.gltf", true);
	auto spaceInvaders = assimpLoad("models/spaceInvaders/scene.gltf", true);
	auto donkeyKong = assimpLoad("models/donkeyKong/scene.gltf", true);

	auto ufo = assimpLoad("models/ufo/scene.gltf", true);

	// loading in floor
	std::vector<Texture> floorTexture = {
		loadTexture("models/arcadeFloor.jpg", "baseTexture")
	};
	auto floorMesh = Mesh3D::square(floorTexture);
	auto floor = Object3D(std::vector<Mesh3D>{floorMesh});
	floor.setScale(glm::vec3(100, 100, 100));
	floor.move(glm::vec3(0, -10, 0));
	floor.rotate(glm::vec3(-M_PI / 2, 0, 0));
	floor.setMaterial(glm::vec4(0.8, 0.8, 0.8, 1.0)); // floor material


	// loading in walls
	std::vector<Texture> wallTexture = {
		loadTexture("models/arcadeWallpaper.png", "baseTexture")
	};
	auto wallMesh = Mesh3D::square(wallTexture);
	auto frontWall = Object3D(std::vector<Mesh3D>{wallMesh});
	frontWall.setScale(glm::vec3(100, 100, 100));
	frontWall.move(glm::vec3(0, 0, -50));
	frontWall.setMaterial(glm::vec4(0.4, 0.4, 0.9, 1.0)); // wall material

	auto leftWall = Object3D(std::vector<Mesh3D>{wallMesh});
	leftWall.setScale(glm::vec3(100, 100, 100));
	leftWall.move(glm::vec3(-50, 0, 0));
	leftWall.rotate(glm::vec3(0, -M_PI / 2, 0));
	leftWall.setMaterial(glm::vec4(0.4, 0.4, 0.9, 1.0)); // so much dark than front and right for some reason

	auto rightWall = Object3D(std::vector<Mesh3D>{wallMesh});
	rightWall.setScale(glm::vec3(100, 100, 100));
	rightWall.move(glm::vec3(50, 0, 0));
	rightWall.rotate(glm::vec3(0, -M_PI / 2, 0));
	rightWall.setMaterial(glm::vec4(0.4, 0.4, 0.9, 1.0));

	auto backWall = Object3D(std::vector<Mesh3D>{wallMesh});
	backWall.setScale(glm::vec3(100, 100, 100));
	backWall.move(glm::vec3(0, 0, 50));
	backWall.setMaterial(glm::vec4(0.4, 0.4, 0.9, 1.0));  // so much darker than front and right for some reason

	// loading in ceiling
	std::vector<Texture> ceilingTexture = {
		loadTexture("models/arcadeCeiling.jpeg", "baseTexture")
	};
	auto ceilingMesh = Mesh3D::square(ceilingTexture);
	auto ceiling = Object3D(std::vector<Mesh3D>{ceilingMesh});
	ceiling.setScale(glm::vec3(100, 100, 100));
	ceiling.rotate(glm::vec3(-M_PI / 2, 0, 0));
	ceiling.move(glm::vec3(0, 30, 0));
	ceiling.setMaterial(glm::vec4(0.8, 0.8, 0.8, 1.0)); // ceiling material

	// bounding room is -50 < x < 50
	//					-10 < y < 30
	//					-50 < z < 50


	// 1
	pacman.setScale(glm::vec3(.14, .14, .14));
	pacman.setPosition(glm::vec3(-35, -10, -35));
	pacman.rotate(glm::vec3(0, M_PI / 4, 0));
	// 2
	finalFight.setScale(glm::vec3(13.5, 13.5, 13.5));
	finalFight.setPosition(glm::vec3(-10, -10, -40));
	// 3 ccw pi/6
	streetFighter.setScale(glm::vec3(16, 16, 16));
	streetFighter.setPosition(glm::vec3(8, 5, -30));
	streetFighter.rotate(glm::vec3(0, M_PI / 6, 0));
	// 4 cw pi/4
	pixelPoro.setScale(glm::vec3(3.5, 3.5, 3.5));
	pixelPoro.setPosition(glm::vec3(37, -10, -34));
	pixelPoro.rotate(glm::vec3(0, -M_PI / 4, 0));
	// 5 cw pi/2
	// make a lil smaller
	//kirby.setScale(glm::vec3(1.2, 1.2, 1.2));
	kirby.setPosition(glm::vec3(40, -10, -12));
	kirby.rotate(glm::vec3(0, -M_PI / 2, 0));
	// 6
	// make a tiny bit bigger
	cyberWing.setScale(glm::vec3(1.1, 1.1, 1.1));
	cyberWing.setPosition(glm::vec3(40, -10, 10));
	// 7 cw 3PI / 4
	ddr.setScale(glm::vec3(4, 4, 4));
	ddr.move(glm::vec3(82, -10, -12));
	ddr.rotate(glm::vec3(0, -(3 * M_PI) / 4, 0));
	// 8 ccw pi / 2
	diablo.setScale(glm::vec3(14, 14, 14));
	diablo.setPosition(glm::vec3(12, 0, 40));
	diablo.rotate(glm::vec3(0, M_PI / 2, 0));
	// 9 ccw 5PI / 6
	rune.setScale(glm::vec3(0.31, 0.31, 0.31));
	rune.move(glm::vec3(-5, -10, 40));
	rune.rotate(glm::vec3(0, (5 * M_PI) / 6, 0));
	// 10 ccw 5PI / 6
	mortalKombat.setScale(glm::vec3(14, 14, 14));
	mortalKombat.setPosition(glm::vec3(-54, 3, 80));
	mortalKombat.rotate(glm::vec3(0, (5 * M_PI) / 6, 0));
	// 11 ccw PI / 4
	spaceInvaders.setScale(glm::vec3(0.065, 0.065, 0.065));
	spaceInvaders.setPosition(glm::vec3(-40, 3, 15));
	spaceInvaders.rotate(glm::vec3(0, M_PI / 4, 0));
	// 12 ccw PI / 2
	donkeyKong.setScale(glm::vec3(3.5, 3.5, 3.5));
	donkeyKong.setPosition(glm::vec3(-40, 0, -15));
	donkeyKong.rotate(glm::vec3(0, M_PI / 2, 0));

	ufo.setScale(glm::vec3(0.06, 0.06, 0.06));
	ufo.move(glm::vec3(0, 20, 0));

	// move machines into objects
	scene.objects.push_back(std::move(ufo));
	scene.objects.push_back(std::move(pacman));
	scene.objects.push_back(std::move(finalFight));
	scene.objects.push_back(std::move(streetFighter));
	scene.objects.push_back(std::move(pixelPoro));
	scene.objects.push_back(std::move(kirby));
	scene.objects.push_back(std::move(cyberWing));
	scene.objects.push_back(std::move(ddr));
	scene.objects.push_back(std::move(diablo));
	scene.objects.push_back(std::move(rune));
	scene.objects.push_back(std::move(mortalKombat));
	scene.objects.push_back(std::move(spaceInvaders));
	scene.objects.push_back(std::move(donkeyKong));

	//Animator animUfo;
	//animUfo.addAnimation(std::make_unique<RotationAnimation>(scene.objects[0], 60, glm::vec3(0, 20 * M_PI, 0)));

	//scene.animators.push_back(std::move(animUfo));

	// moving floor into scene
	scene.objects.push_back(std::move(floor));
	// moving walls into scene
	scene.objects.push_back(std::move(frontWall));
	scene.objects.push_back(std::move(leftWall));
	scene.objects.push_back(std::move(rightWall));
	scene.objects.push_back(std::move(backWall));
	// moving ceiling into scene
	scene.objects.push_back(std::move(ceiling));

	return scene;

}


int main() {
	
	std::cout << std::filesystem::current_path() << std::endl;
	
	// Initialize the window and OpenGL.
	sf::ContextSettings settings;
	settings.depthBits = 24; // Request a 24 bits depth buffer
	settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	settings.antialiasingLevel = 2;  // Request 2 levels of antialiasing
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	sf::Window window(sf::VideoMode{ 1200, 800 }, "Modern OpenGL", sf::Style::Resize | sf::Style::Close, settings);
	
	sf::Music backgroundMusic; // SMFL Audio
	if (!backgroundMusic.openFromFile("models/retroMusic.wav")) {
		std::cerr << "Error loading music file" << std::endl;
		return 1;
	}

	// play music in loop
	backgroundMusic.setLoop(true);
	backgroundMusic.play();

	gladLoadGL();
	glEnable(GL_DEPTH_TEST);

	// Inintialize scene objects.
	auto myScene = arcadeScene();
	// You can directly access specific objects in the scene using references.
	/*auto& firstObject = myScene.objects[0];*/

	// Activate the shader program.
	myScene.program.activate();

	// Camera parameters initialized outside of while loop to update inside for user interaction
	glm::vec3 cameraPos(0, 0, 5);
	glm::vec3 cameraFront(0, 0, -1); // looking down negative z at first
	glm::vec3 cameraUp(0, 1, 0);
	float cameraSpeed = 10; // movement speed
	float pitch = 0; // up-down rotation
	float yaw = -90; // left-right rotation

	glm::vec3 ambientColor(0.5, 0.5, 0.5);

	//glm::vec3 directionalLight1(0, -1, 0); // light pointing straight down
	glm::vec3 directionalLight(0, 0, -1); // facing straight forward rn
	glm::vec3 directionalColor(1, 1, 0.8); 

	myScene.program.setUniform("ambientColor", ambientColor);
	//myScene.program.setUniform("directionalLight", directionalLight);
	myScene.program.setUniform("directionalColor", directionalColor);

	
	// Ready, set, go!
	bool running = true;
	sf::Clock c;
	auto last = c.getElapsedTime();

	// Start the animators.
	for (auto& anim : myScene.animators) {
		anim.start();
	}

	//// set material uniforms for each object in scene
	//for (auto& o : myScene.objects) {
	//	// object material uniforms
	//	myScene.program.setUniform("material", o.getMaterial());
	//}


	while (running) {
		
		sf::Event ev;
		while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
				running = false;
			}
		}
		auto now = c.getElapsedTime();
		auto diff = now - last;
		float dt = diff.asSeconds(); // for user interaction
		last = now;


		// WASD input for camera movement
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { // move forward
			cameraPos += cameraSpeed * dt * cameraFront;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { // move backwards
			cameraPos -= cameraSpeed * dt * cameraFront;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { // move left
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * dt;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { // move right
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * dt;
		}

		// arrow key input for camera look around
		float sens = 30 * dt;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { // look up
			pitch += sens;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { // look down
			pitch -= sens;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { // look left
			yaw -= sens;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { // look right
			yaw += sens;
		}


		// update cameraFront
		glm::vec3 front;
		//x = cos(yaw) * cos(pitch)
		//y = sin(pitch)
		//z = sin(yaw) * cos(pitch)
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		// spotlight uniforms
		//myScene.program.setUniform("")

		// update view and projection matrix
		glm::mat4 camera = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); // position, target, and up vector = new view matrix
		glm::mat4 perspective = glm::perspective(glm::radians(45.0), static_cast<double>(window.getSize().x) / window.getSize().y, 0.1, 100.0);
		
		// give shader programs uniform inputs inside loop to update camera
		myScene.program.setUniform("view", camera);
		myScene.program.setUniform("projection", perspective);
		//myScene.program.setUniform("cameraPos", cameraPos);
		myScene.program.setUniform("viewPos", cameraPos);
		myScene.program.setUniform("directionalLight", cameraFront);


		// Update the scene.
		for (auto& anim : myScene.animators) {
			anim.tick(dt);
		}

		// Clear the OpenGL "context".
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render the scene objects.
		for (auto& o : myScene.objects) {
			// object material uniforms
			myScene.program.setUniform("material", o.getMaterial());
			o.render(myScene.program);
		}
		window.display();

	}

	return 0;
}