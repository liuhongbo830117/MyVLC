#pragma once
#include "Communicator.h"
class SplitAmplitudeCommunicator :
	public Communicator
{
public:
	////////////////////////////// Split Amplitude ///////////////////////////

	// symbol_time: how many milliseconds will the symbol last
	void sendImage(double frequency, string inputImage, string msg, string outputVideoFile, int symbol_time)
	{
		Mat img = imread(inputImage);

		int framerate = frequency; //get the frame rate
		int frame_width = img.cols;
		int frame_height = img.rows;
		//int frames_per_symbol = (framerate * 1000) / symbol_time; // symbol time in milliseconds and framerate in frames per second
		double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
		vector<float> amplitudes1 = createWaveGivenFPS(frequency, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin1);
		double lumin2[] = { LUMINANCE[1], LUMINANCE[0] };
		vector<float> amplitudes2 = createWaveGivenFPS(frequency, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin2);
		// create the video writer
		ostringstream outputVideoStream;
		outputVideoStream << msg << "_AmpDiff" << Utilities::createOuputVideoName(symbol_time, "image", outputVideoFile);
		VideoWriter vidWriter = Utilities::getVideoWriter(outputVideoStream.str(), framerate, Utilities::getFrameSize());
		for (int i = 0; i < amplitudes1.size(); i++)
		{
			Mat frame;
			cv::resize(img, frame,Utilities::getFrameSize());
			Utilities::updateFrameWithAlpha(frame, cv::Rect(0, 0, frame.cols / 2, frame.rows), amplitudes1[i]);
			Utilities::updateFrameWithAlpha(frame, cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows), amplitudes2[i]);
			vidWriter << frame;
		}
	}

	// symbol_time: how many milliseconds will the symbol last
	void sendVideo(string inputVideoFile, string msg, string outputVideoFile, int symbol_time)
	{
		VideoCapture videoReader(inputVideoFile);
		if (videoReader.isOpened())
		{
			videoReader.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
			int framerate = videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (frame_height > 1000)
			{
				frame_width /= 2;
				frame_height /= 2;
			}
			int fps = Utilities::lcm((int)framerate, Utilities::lcm(2 * FREQ[ZERO], 2 * FREQ[ONE]));
			//int frames_per_symbol = (fps * 1000) / symbol_time; // symbol time in milliseconds and framerate in frames per second
			double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
			vector<float> amplitudes1 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin1);
			double lumin2[] = { LUMINANCE[1], LUMINANCE[0] };
			vector<float> amplitudes2 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin2);
			Mat frame;
			// create the video writer
			ostringstream outputVideoStream;
			outputVideoStream << msg << "_AmpDiff" << Utilities::createOuputVideoName(symbol_time, inputVideoFile, outputVideoFile);
			VideoWriter vidWriter = Utilities::getVideoWriter(outputVideoStream.str(), fps, Utilities::getFrameSize());
			int inputFrameUsageFrames = fps / framerate;
			for (int k = 0; k < amplitudes1.size(); k++)
			{
				if (k%inputFrameUsageFrames == 0)
				{
					videoReader.read(frame);
				}
				Mat tmp;
				cv::resize(frame, tmp, Utilities::getFrameSize());
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(0, 0, tmp.cols / 2, tmp.rows), amplitudes1[k]);
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(tmp.cols / 2, 0, tmp.cols / 2, tmp.rows), amplitudes2[k]);
				vidWriter << tmp;

			}
		}
		cout << endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	///              //////////////      Receive     ///////////////                         ////
	/////////////////////////////////////////////////////////////////////////////////////////////


	// receive with a certain ROI ratio
	void receive(string fileName, int frames_per_symbol, double ROI_Ratio)
	{
		int fps = 0;
		vector<vector<float> > frames = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps, 2);
		vector<float> amplitude_difference;
		for (int i = 0; i < frames[0].size(); i++)
		{
			amplitude_difference.push_back(frames[0][i] - frames[1][i]);
		}
		receive2(amplitude_difference, fps, frames_per_symbol);
	}
};

