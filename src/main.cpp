#include "main.h"



static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void gRenderInit()
{
	grender.SetCallbackFunctions();
	grender.compileAndLinkShader();
	grender.setColorSize(colorWidth, colorHeight);

	grender.setLocations();
	grender.setVertPositions();
	grender.allocateBuffers();
	grender.setTextures(gflow.getColorTexture(), gflow.getEdgesTexture()); //needs texture uints from gfusion init
	grender.anchorMW(std::make_pair<int, int>(1920 - 512 - grender.guiPadding().first, grender.guiPadding().second));
	//krender.genTexCoordOffsets(1, 1, 1.0f);
}

void searchForMedia()
{
	videosFromFile.resize(0);
	imagesFromFile.resize(0);

	cv::String pathVideos("videos/*.wmv"); //select only jpg
	//cv::String pathVideos("videos/*.mkv"); //select only jpg

	std::vector<cv::String> fnVideos;
	cv::glob(pathVideos, fnVideos, true); // recurse



	for (size_t k = 0; k<fnVideos.size(); ++k)
	{
		std::cout << fnVideos[k] << std::endl;

		cv::VideoCapture cap(fnVideos[k]);
		cap.set(CV_CAP_PROP_BUFFERSIZE, 5);
		if (!cap.isOpened())
		{
			std::cout << "cannot open video file" << std::endl;
			//return;
		}

		videosFromFile.push_back(cap);
	}

	//cv::VideoWriter outWriter;
	//outWriter.open("output/output.wmv", static_cast<int>(videosFromFile[0].get(CV_CAP_PROP_FOURCC)), 30, cv::Size(1920, 1080), true);
	//if (!outWriter.isOpened())
	//{
	//	std::cout << "Could not open the output video for write" << std::endl;
	//	//return -1;
	//}

	cv::String pathImages("images/*.png"); //select only jpg
	std::vector<cv::String> fnImages;
	cv::glob(pathImages, fnImages, true); // recurse
	for (size_t k = 0; k<fnImages.size(); ++k)
	{
		std::cout << fnImages[k] << std::endl;

		cv::Mat im = cv::imread(fnImages[k]);
		if (im.empty())
		{
			std::cout << "empty image " << std::endl;
			continue; //only proceed if sucsessful
		}					  // you probably want to do some preprocessing
							  //if (k == 0)
							  //imagesFromFile.push_back(cv::Mat(im.rows, im.cols, CV_8UC3));
		imagesFromFile.push_back(im);
	}
}


void resetFlowSize()
{
	gflow.firstFrame = true;
	//gflow.clearTexturesAndBuffers();
	gflow.setNumLevels(colorWidth);
	gflow.setTextureParameters(colorWidth, colorHeight);
	gflow.allocateTextures(false);
	gflow.allocateBuffers();

	grender.setColorSize(colorWidth, colorHeight);

	//fGrabber.setImageDimensions(colorWidth, colorHeight);
	//fGrabber.resetImageDimensions();

	/*cap.release();

	if (cap.open(0)) 
	{
		cap.set(CV_CAP_PROP_FRAME_WIDTH, resoPresetPair[resoPreset].first);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, resoPresetPair[resoPreset].second);
	}*/


	changedSource = false;
}




int main(int, char**)
{


	int display_w, display_h;
	// load openGL window
	window = grender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	resoPresetPair.push_back(std::make_pair(640, 480));
	resoPresetPair.push_back(std::make_pair(960, 540));
	resoPresetPair.push_back(std::make_pair(1280, 720));
	resoPresetPair.push_back(std::make_pair(1920, 1080));

	colorWidth = resoPresetPair[resoPreset].first;
	colorHeight = resoPresetPair[resoPreset].second;

	gflow.setupEKF();
	// op flow init
	gflow.compileAndLinkShader();
	gflow.setLocations();

	gflow.setNumLevels(colorWidth);
	gflow.setTextureParameters(colorWidth, colorHeight);
	gflow.allocateTextures(false);

	gflow.allocateBuffers();

	gRenderInit();


	camera.start();
	//camera.setImageDimensions(colorWidth, colorHeight);


	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);
		grender.renderScaleHeight((float)display_h / 1080.0f);
		grender.renderScaleWidth((float)display_w / 1920.0f);

		grender.anchorMW(std::make_pair<int, int>(50, 1080 - 424 - 50));

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);


		//cv::cvtColor(col, newcol, CV_RGB2RGBA, 4);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();


		bool frameReady = camera.frames(colorArray.data(), NULL, NULL, NULL, NULL);
		if (frameReady)
		{
			gflow.setTexture(colorArray.data());

			gflow.calc(false);

		}
		else
		{
			//std::cout << "asasd!" << std::endl;
		}
		grender.setTrackedPointsBuffer(gflow.getTrackedPointsBuffer());

		grender.setFlowTexture(gflow.getFlowTexture());

		grender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag);

		grender.setColorImageRenderPosition(vertFov);

		grender.setFlowImageRenderPosition(colorHeight, colorWidth, vertFov);

		grender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
		grender.setProjectionMatrix();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		grender.Render(false);





		if (grender.showImgui())
		{
			ImGui::SetNextWindowPos(ImVec2(1600 - 32 - 528 - 150, 32));
			ImGui::SetNextWindowSize(ImVec2(528 + 150, 424), ImGuiSetCond_Always);
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoTitleBar;
			//window_flags |= ImGuiWindowFlags_ShowBorders;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoCollapse;

			float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			arr[0] = gflow.getTimeElapsed();
			arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];
			GLint total_mem_kb = 0;
			glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
				&total_mem_kb);

			GLint cur_avail_mem_kb = 0;
			glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
				&cur_avail_mem_kb);

			bool showGUI = grender.showImgui();
			ImGui::Begin("Menu", &showGUI, window_flags);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);
			ImGui::Text("GPU Memory Usage %d MB out of %d (%.1f %%)", (total_mem_kb - cur_avail_mem_kb) / 1024, total_mem_kb / 1024, 100.0f * (1.0f - (float)cur_avail_mem_kb / (float)total_mem_kb));




			//ImGui::PushItemWidth(-krender.guiPadding().first);
			//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
			ImGui::Text("Help menu - press 'H' to hide");
			ImGui::Separator();
			ImGui::Text("Scan for media");
			if (ImGui::Button("Scan")) searchForMedia();





			ImGui::Separator();
			ImGui::Text("View Options");

			if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);

			if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);

			ImGui::Separator();
			ImGui::Text("Other Options");

			if (ImGui::Button("Reset flow points")) gflow.clearPoints();
			//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

			//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

			//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
			//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
			//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


			ImGui::Separator();
			ImGui::Text("View Transforms");
			ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);

			ImGui::SliderFloat("valA", &valA, 0.00001f, 0.5f);
			ImGui::SliderFloat("valB", &valB, 0.00001f, 0.5f);

			grender.setFov(vertFov);
			gflow.setVals(valA, valB);

			if (ImGui::Button("Reset Sliders")) resetSliders();




			ImGui::End();






			ImGui::Render();




			//grender.setComputeWindowPosition();
			//gfusion.render();
		}
		glfwSwapBuffers(window);

	}

	// Cleanup DO SOME CLEANING!!!
	ImGui_ImplGlfwGL3_Shutdown();


	//krender.cleanUp();

	return 0;
}