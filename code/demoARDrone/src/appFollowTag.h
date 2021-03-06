// Copyright (C) 2013 by - FZI Forschungszentrum Informatik am Karlsruher Institut fuer Technologie
//                       - Institut Eurécom
//                       - Télécom ParisTech
// 
// Author: Benjamin Ranft (benjamin.ranft@web.de)
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


// follow a tag, which is e.g. carried by a person
// ===============================================

#pragma once

#include "appBase.h"
#include "appDevel.h"
#include "hawaii/common/tracker.h"
#include "hawaii/common/helpers.h" //ADDTION MADE
#include <opencv2/imgproc/imgproc.hpp>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_joystick.h>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string> // this should be already included in <sstream>
using namespace std;

int counter;
// derived from base drone application
class DroneAppFollowTag : public DroneAppBase {
	
	// c'tor to set camera parameters
	public:
	DroneAppFollowTag() :
		DroneAppBase( false ),
		trkDistanceX( 0.3 ),
		trkDistanceZ( 0.3 ),
		trkAngleZ(    0.3 ) {
		#if 1
			this->commands.controllerLinX.kI = 0.0 ;
			this->commands.controllerLinY.kI = 0.0 ;
			this->commands.controllerLinZ.kI = 0.0 ;
			this->commands.controllerAngZ.kI = 0.0 ;
		#endif
	}
	virtual void processImageFront( const cv_bridge::CvImage imageFront ) override {
		cv::Mat visualization = imageFront.image ;
		hawaii::AutoMat imageBGR, imageGray, imageGraySmooth ;
		imageFront.image.copyTo( imageBGR.writeCPU( imageFront.image.size(), imageFront.image.type() ) ) ;
		string Result;          // string which will contain the result
		ostringstream convert;   // stream used for the conversion
		convert << counter;      // insert the textual representation of 'counter' in the characters in the stream
		Result = convert.str(); // set 'Result' to the contents of the stream
		imwrite( "/home/drone/repos/code/demoARDrone/src/mydemo/"+Result+".jpg", visualization );
		counter ++;
		HAWAII_IMSHOW( visualization ) ;
		
		cv::waitKey( 1 );
                // printf("VISUALIZATION IS SHOWED");
		}
	// rely on drone SDK's tag detection, therefore only use "navdata"
	protected:
	hawaii::common::Tracker1DPT1 trkDistanceX,
	                             trkDistanceZ,
	                             trkAngleZ    ;
	hawaii::common::Timer::Rep timeLastSeen ;
	virtual void processNavdata( const ardrone_autonomy::Navdata navdata ) override {
		
		// follow tag
		if( navdata.tags_count > 0 ) {
			
			// keep distance to tag via pitch
			this->trkDistanceX( - navdata.tags_distance[ 0 ] / 100.0 ) ;
			this->commands.controlLinX( -1.5 - this->trkDistanceX ) ;
			printf( "distanceX = %5.2f, ctrl = %5.2f\n", (double)this->trkDistanceX, this->commands.movement.linear.x ) ;
			
			// change height if tag is up or down
			const double horizonV = this->principalPointFrontV - sin( navdata.rotY * CV_PI / 180.0 ) * this->focalLengthFrontV ;
			this->trkDistanceZ( ( navdata.tags_yc[ 0 ] / 1000.0 * 360.0 - horizonV ) / this->focalLengthFrontV * navdata.tags_distance[ 0 ] / 100.0 ) ;
			this->commands.controlLinZ( 0.0 - this->trkDistanceZ ) ;
			printf( "distanceZ = %5.2f, ctrl = %5.2f\n", (double)this->trkDistanceZ, this->commands.movement.linear.z ) ;
			
			// turn if tag is left or right
			this->trkAngleZ( atan2( navdata.tags_xc[ 0 ] / 1000.0 * 640.0 - this->principalPointFrontU, this->focalLengthFrontU ) ) ;
			this->commands.controlAngZ( 0.0 - this->trkAngleZ ) ;
			printf( "angleZ = %5.2f, ctrl = %5.2f\n\n", (double)this->trkAngleZ * 180.0 / CV_PI, this->commands.movement.angular.z ) ;
			
			this->timeLastSeen = hawaii::common::Timer::timeSinceEpoch() ;
		}
		
		// simply hover after tag has not been seen for a while
		else {
			if( ( hawaii::common::Timer::timeSinceEpoch() - this->timeLastSeen ) * hawaii::common::Timer::factorRepToMilliseconds < 500.0 ) {
				printf( "tag not found, continuing...\n\n" ) ;
			} else {
				this->commands = DroneCommands() ;//comment: call hover(); is enough
				printf( "tag still not found, hovering...\n\n" ) ;
			}
		}
		
	} // method "DroneAppFollowTag::processNavdata()"
	
} ; // class "DroneAppFollowTag"
