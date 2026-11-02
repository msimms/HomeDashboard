//
//  DashboardAppApp.swift
//  Created by Michael Simms on 2/7/26.
//

import SwiftUI

@main
struct DashboardAppApp: App {
	var serverVM: ServerVM = ServerVM.shared

	var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

