#include "interface.h"

//#include <libfreenect2/packet_pipeline.h>




void Realsense2Camera::start()
{
	if (m_status == STOPPED)
	{
		m_status = CAPTURING;
		m_thread = new std::thread(&Realsense2Camera::captureLoop, this);
	}
}

void Realsense2Camera::stop()
{
	if (m_status == CAPTURING)
	{
		m_status = STOPPED;

		if (m_thread->joinable())
		{
			m_thread->join();
		}

		m_thread = nullptr;
	}
}

void Realsense2Camera::frames(unsigned char * colorAray, float * bigDepthArray)
{
	//m_mtx.lock();

	//memcpy_S the arrays to the pointers passed in here

	memcpy_s(colorAray, 1920 * 1080 * 4, m_rawColor, 1920 * 1080 * 4);
	memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);



	//m_mtx.unlock();
	m_frames_ready = false;
}

// get all the frames available, pass a null pointer in for each array you dont want back
bool Realsense2Camera::frames(uint8_t * colorArray, uint16_t * depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping)
{

	//memcpy_S the arrays to the pointers passed in here

	if (m_frames_ready)
	{
		m_mtx.lock();

		if (colorArray != NULL)
		{
			memcpy_s(colorArray, m_colorframe_width * m_colorframe_height * 4, m_color_frame, m_colorframe_width * m_colorframe_height * 4);
		}

		if (depthArray != NULL)
		{
			memcpy_s(depthArray, m_depthframe_width * m_depthframe_height * 2, m_depth_frame, m_depthframe_width * m_depthframe_height * 2);
		}

		if (infraredArray != NULL)
		{
			//memcpy_s(infraredArray, 512 * 424 * 4, m_infra_frame, 512 * 424 * 4);
		}

		if (bigDepthArray != NULL)
		{
			//memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);
		}
		if (colorDepthMapping != NULL)
		{
			//memcpy_s(colorDepthMapping, 512 * 424 * 4, m_color_Depth_Map, 512 * 424 * 4);
		}
		m_frames_ready = false;
		m_mtx.unlock();
		return true;
	}
	else
	{
		return false;
	}


}


std::vector<float> Realsense2Camera::getColorCameraParameters()
{
	std::vector<float> camPams;

	/*camPams.push_back(m_colorCamPams.fx);
	camPams.push_back(m_colorCamPams.fy);
	camPams.push_back(m_colorCamPams.cx);
	camPams.push_back(m_colorCamPams.cy);
	camPams.push_back(m_colorCamPams.mx_x3y0);
	camPams.push_back(m_colorCamPams.mx_x0y3);
	camPams.push_back(m_colorCamPams.mx_x2y1);
	camPams.push_back(m_colorCamPams.mx_x1y2);
	camPams.push_back(m_colorCamPams.mx_x2y0);
	camPams.push_back(m_colorCamPams.mx_x0y2);
	camPams.push_back(m_colorCamPams.mx_x1y1);
	camPams.push_back(m_colorCamPams.mx_x1y0);
	camPams.push_back(m_colorCamPams.mx_x0y1);
	camPams.push_back(m_colorCamPams.mx_x0y0);
	camPams.push_back(m_colorCamPams.my_x3y0);
	camPams.push_back(m_colorCamPams.my_x0y3);
	camPams.push_back(m_colorCamPams.my_x2y1);
	camPams.push_back(m_colorCamPams.my_x1y2);
	camPams.push_back(m_colorCamPams.my_x2y0);
	camPams.push_back(m_colorCamPams.my_x0y2);
	camPams.push_back(m_colorCamPams.my_x1y1);
	camPams.push_back(m_colorCamPams.my_x1y0);
	camPams.push_back(m_colorCamPams.my_x0y1);
	camPams.push_back(m_colorCamPams.my_x0y0);
	camPams.push_back(m_colorCamPams.shift_d);
	camPams.push_back(m_colorCamPams.shift_m);*/

	return camPams;
}

void Realsense2Camera::captureLoop()
{
	//m_frame_width = 512;
	//m_frame_height = 424;

	//m_rawColor = new float[1920 * 1080];
	//m_rawBigDepth = new float[1920 * 1082];

	m_color_frame = new float[m_colorframe_width * m_colorframe_height * 3];
	m_depth_frame = new uint16_t[m_depthframe_width * m_depthframe_height];
	//m_infra_frame = new float[m_frame_width * m_frame_height];
	//m_color_Depth_Map = new int[m_frame_width * m_frame_height];


	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;

	//Add desired streams to configuration
	cfg.enable_stream(RS2_STREAM_COLOR, m_colorframe_width, m_colorframe_height, RS2_FORMAT_BGRA8, 60);
	cfg.enable_stream(RS2_STREAM_DEPTH, m_depthframe_width, m_depthframe_height, RS2_FORMAT_Z16, 90);


	// Start streaming with default recommended configuration
	rs2::pipeline_profile selection = pipe.start(cfg);

	rs2::frameset data; 

	auto depth_stream = selection.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();

	auto i = depth_stream.get_intrinsics();
	m_depth_ppx = i.ppx;
	m_depth_ppy = i.ppy;
	m_depth_fx = i.fx; 
	m_depth_fy = i.fy;



	rs2::frame depth;
	rs2::frame color;

	while (m_status == CAPTURING)
	{
		data = pipe.wait_for_frames(); // Wait for next set of frames from the camera

		depth = data.get_depth_frame(); // Find and colorize the depth data
		color = data.get_color_frame();            // Find the color data

												   // Query frame size (width and height)
		const int w = color.as<rs2::video_frame>().get_width();
		const int h = color.as<rs2::video_frame>().get_height();

		const int wD = depth.as<rs2::video_frame>().get_width();
		const int hD = depth.as<rs2::video_frame>().get_height();

		//cv::Mat colMat = cv::Mat(hD, wD, CV_16SC1, (void*)depth.get_data());

		//cv::imshow("cv wsrtyindow", colMat);
		//cv::waitKey(1);
		m_mtx_inner.lock();
		memcpy_s(m_color_frame, w * h * 4, color.get_data(), w * h * 4);
		memcpy_s(m_depth_frame, wD * hD * 2, depth.get_data(), wD * hD * 2);

		m_frames_ready = true;
		m_mtx_inner.unlock();


	}

	pipe.stop();

	delete m_rawColor;
	delete m_rawBigDepth;

	delete m_color_frame;
	delete m_depth_frame;
	delete m_infra_frame;

	delete m_color_Depth_Map;

}
