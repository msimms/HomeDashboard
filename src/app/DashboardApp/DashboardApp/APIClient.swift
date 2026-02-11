//
//  APIClient.swift
//  Created by Michael Simms on 2/7/26.
//

import Foundation

struct IndoorData: Decodable {
	let co2_ppm: Int
	let temp_c: Float
	let humidity: Float
	let voc: Int
	let voc_index: Int
	let ts: Float

	enum CodingKeys: String, CodingKey {
		case co2_ppm
		case temp_c
		case humidity
		case voc
		case voc_index
		case ts
	}
}

struct PatioData: Decodable {
	let temp_c: Float
	let humidity: Float
	let moisture_sensor_1: Float
	let moisture_sensor_2: Float
	let ts: Float

	enum CodingKeys: String, CodingKey {
		case temp_c
		case humidity
		case moisture_sensor_1
		case moisture_sensor_2
		case ts
	}
}

enum APIError: Error { case badURL, badResponse }

struct APIClient {
	static func fetchIndoorStatus() async throws -> IndoorData {
		guard let url = URL(string: "https://status.mikesimms.net/api/1.0/indoor_air?latest") else {
			throw APIError.badURL
		}
		let (data, response) = try await URLSession.shared.data(from: url)
		guard let http = response as? HTTPURLResponse, (200..<300).contains(http.statusCode) else {
			throw APIError.badResponse
		}
		return try JSONDecoder().decode(IndoorData.self, from: data)
	}

	static func fetchPatioStatus() async throws -> PatioData {
		guard let url = URL(string: "https://status.mikesimms.net/api/1.0/patio?latest") else {
			throw APIError.badURL
		}
		let (data, response) = try await URLSession.shared.data(from: url)
		guard let http = response as? HTTPURLResponse, (200..<300).contains(http.statusCode) else {
			throw APIError.badResponse
		}
		return try JSONDecoder().decode(PatioData.self, from: data)
	}
}
