#pragma once

#include "Utilities.h"
#include "SplitFrequencyCommunicator.h"

class SplitFrequencyAmplitudeCommunicator : public SplitFrequencyCommunicator
{
public:
	
	
	////////////////////////////////// Split Frequency and Amplitude ////////////////////////////////

	// symbol_time: how many milliseconds will the symbol last
	void sendImage(double frequency, string inputImage, string msg, string outputVideoFile, int symbol_time)
	{
		Mat img = imread(inputImage);

		int fps = frequency; //get the frame rate
		int frame_width = img.cols;
		int frame_height = img.rows;
		//int frames_per_symbol = (framerate * 1000) / symbol_time; // symbol time in milliseconds and framerate in frames per second
		double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
		double lumin2[] = { LUMINANCE[1], LUMINANCE[0] };
		vector<float> amplitudes11 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin1);
		vector<float> amplitudes12 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ONE], FREQ[ZERO], lumin1);
		vector<float> amplitudes21 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin2);
		vector<float> amplitudes22 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ONE], FREQ[ZERO], lumin2);
		// create the video writer
		ostringstream outputVideoStream;
		outputVideoStream << msg << "_FreqAmpDiff" << Utilities::createOuputVideoName(symbol_time, "image", outputVideoFile);
		VideoWriter vidWriter = Utilities::getVideoWriter(outputVideoStream.str(), fps, Utilities::getFrameSize());
		for (int i = 0; i < amplitudes11.size(); i++)
		{
			Mat frame;
			cv::resize(img, frame, Utilities::getFrameSize());
			Utilities::updateFrameWithAlpha(frame, cv::Rect(0, 0, frame.cols / 2, frame.rows / 2), amplitudes11[i]);
			Utilities::updateFrameWithAlpha(frame, cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows / 2), amplitudes12[i]);
			Utilities::updateFrameWithAlpha(frame, cv::Rect(0, frame.rows / 2, frame.cols / 2, frame.rows / 2), amplitudes21[i]);
			Utilities::updateFrameWithAlpha(frame, cv::Rect(frame.cols / 2, frame.rows / 2, frame.cols / 2, frame.rows / 2), amplitudes22[i]);
			vidWriter << frame;
		}
	}

	void sendVideo(string inputVideoFile, string msg, string outputVideoFile, int symbol_time)
	{
		VideoCapture videoReader(inputVideoFile);
		if (videoReader.isOpened())
		{
			videoReader.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
			int framerate = videoReader.get(CV_CAP_PROP_FPS); //get the frame rate
			int frame_width = videoReader.get(CV_CAP_PROP_FRAME_WIDTH);
			int frame_height = videoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
			int fps = Utilities::lcm((int)framerate, Utilities::lcm(2 * FREQ[ZERO], 2 * FREQ[ONE]));
			//int frames_per_symbol = (fps * 1000) / symbol_time; // symbol time in milliseconds and framerate in frames per second
			double lumin1[] = { LUMINANCE[0], LUMINANCE[1] };
			double lumin2[] = { LUMINANCE[1], LUMINANCE[0] };
			vector<float> amplitudes11 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin1);
			vector<float> amplitudes12 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ONE], FREQ[ZERO], lumin1);
			vector<float> amplitudes21 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ZERO], FREQ[ONE], lumin2);
			vector<float> amplitudes22 = createWaveGivenFPS(fps, msg, symbol_time, FREQ[ONE], FREQ[ZERO], lumin2);
			Mat frame;
			// create the video writer
			ostringstream outputVideoStream;
			outputVideoStream << msg << "_FreqAmpDiff" << Utilities::createOuputVideoName(symbol_time, inputVideoFile, outputVideoFile);
			VideoWriter vidWriter = Utilities::getVideoWriter(outputVideoStream.str(), fps, Utilities::getFrameSize());
			int inputFrameUsageFrames = fps / framerate;
			for (int k = 0; k < amplitudes11.size(); k++)
			{
				if (k%inputFrameUsageFrames == 0)
				{
					videoReader.read(frame);
				}
				Mat tmp;
				cv::resize(frame, tmp, Utilities::getFrameSize());
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(0, 0, tmp.cols / 2, tmp.rows / 2), amplitudes11[k]);
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(tmp.cols / 2, 0, tmp.cols / 2, tmp.rows / 2), amplitudes12[k]);
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(0, tmp.rows / 2, tmp.cols / 2, tmp.rows / 2), amplitudes21[k]);
				Utilities::updateFrameWithAlpha(tmp, cv::Rect(tmp.cols / 2, tmp.rows / 2, tmp.cols / 2, tmp.rows / 2), amplitudes22[k]);
				vidWriter << tmp;

			}
		}
		cout << endl;
	}

	// receive with a certain ROI ratio
	void receive(string fileName, int frames_per_symbol, double ROI_Ratio)
	{
		int fps = 0;
		vector<vector<float> > temp = Utilities::getVideoFrameLuminancesSplitted(fileName, ROI_Ratio, fps, 4);
		vector<vector<float> > frames(2);
		for (int i = 0; i < temp[0].size(); i++)
		{
			frames[0].push_back(temp[0][i] - temp[2][i]);
			frames[1].push_back(temp[1][i] - temp[3][i]);
		}
		receiveWithInputROIRatioFreqDiff(frames, fps, frames_per_symbol);
	}
};
