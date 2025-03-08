//
//  StringUtils.swift
//  Created by Michael Simms on 3/7/25.
//

import Foundation

class StringUtils {

	/// @brief Utility function for converting a number of seconds into HH:MMSS format
	static func formatAsHHMMSS(numSeconds: Double) -> String {
		let SECS_PER_DAY  = 86400
		let SECS_PER_HOUR = 3600
		let SECS_PER_MIN  = 60
		var tempSeconds   = Int(numSeconds)
		
		let days     = (tempSeconds / SECS_PER_DAY)
		tempSeconds -= (days * SECS_PER_DAY)
		let hours    = (tempSeconds / SECS_PER_HOUR)
		tempSeconds -= (hours * SECS_PER_HOUR)
		let minutes  = (tempSeconds / SECS_PER_MIN)
		tempSeconds -= (minutes * SECS_PER_MIN)
		let seconds  = (tempSeconds % SECS_PER_MIN)
		
		if days > 0 {
			return String(format: "%02d:%02d:%02d:%02d", days, hours, minutes, seconds)
		}
		else if hours > 0 {
			return String(format: "%02d:%02d:%02d", hours, minutes, seconds)
		}
		return String(format: "%02d:%02d", minutes, seconds)
	}
}

