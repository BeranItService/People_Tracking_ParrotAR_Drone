//  // Copyright (C) 2013 by: - Institut Eurécom
//                            - Télécom ParisTech
// 
//Author: Aakanksha Rana (rana.aakanksha@gmail.com) and Praveer Singh (praveersingh1990@gmail.com)
// 
// This file is part of demoARDrone.
// 
// demoARDrone is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later 
// version.
// 
// demoARDrone is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with libHawaii. If not, see 
// <http://www.gnu.org/licenses/>.


// monocular sparse 3D reconstruction (via visual odometry)
// ========================================================

#pragma once

#include "commands.h"
#include "hawaii/common/timer.h"
#include "hawaii/common/tracker.h"
#include <opencv2/core/core.hpp>
#include <memory>

class VisualOdometryMono;


class appFollowPersonFP {
	
	// c'tor with camera properties
	// user note: Before passing images, you need to undistort and stretch them such that the focal lengths along the u- 
	//            and v-axes are identical. "DroneAppBase" already does that.
	public:
	appFollowPersonFP( const double focalLength,
	          const double principalPointU,
	          const double principalPointV ) ;
	
	protected:

	
	// process the latest image, together with the drone's on-board odometry pose at the time is was captured
	// user note: If available, pass a grayscale image to avoid internal conversion.
	public:
	bool processImageFront( const cv::Mat   imagefr,
	                        const cv::Vec3d rotationGlobal,
	                        const cv::Vec3d translationGlobal) ;
	protected:
	cv::Size sizeImage ;
	double scaleFactor ;
	
	// get control commands: See flight parameters above. Vertical and sideways motion are always set - therefore "true" 
	//                       is always returned. Forward and yaw motion occur only after successful 3D reconstruction.
		
	public:
	
	bool getCommands(DroneCommands& commands, const cv::Vec3d rotationGlobal,
	                  const cv::Vec3d      translationGlobal ) const ;
	
	protected:
	mutable hawaii::common::Tracker1DPT1 trackerAngleDelta,
	                                     trackerForwardSpeed ;
	
	// wrap visual odometry implementation via "pimpl" pattern, results of previous calls
	// developer note: "std::auto_ptr" is officially deprecated, but "boost::scoped_ptr" or "std::unique_ptr" would 
	//                 require the d'tor of "impl" to be defined (!) in the source file of this class. This sucks...
	protected:
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        std::auto_ptr< VisualOdometryMono > visoPtr ;
	#pragma GCC diagnostic pop
	bool   visoSuccessPrev ;
	size_t visoFailsConsecMax,
	       visoFailsConsec    ;
	
	// odometry based on drone's additional sensors as a second guess for visual odometry scale estimation, pose of 
	// previous image successfully used by visual odometry
	protected:
	cv::Vec3d rotationGlobalOnboardPrev,
	          translationGlobalOnboardPrev ;
	
	//-----------------------variable for drift computation-------------------------//
       private:
       
       bool start_defined;
       double start_yaw;
       cv::Vec3d start_translation;
       short img_taken;
       //-----------------------variable for drift computation-------------------------//	
	
} ; // class "appFollowPersonFP"
