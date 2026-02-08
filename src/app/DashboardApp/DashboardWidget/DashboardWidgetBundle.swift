//
//  DashboardWidgetBundle.swift
//  Created by Michael Simms on 2/7/26.
//

import WidgetKit
import SwiftUI

@main
struct DashboardWidgetBundle: WidgetBundle {
	var body: some Widget {
		DashboardWidget()
		DashboardWidgetControl()
		DashboardWidgetLiveActivity()
	}
}
