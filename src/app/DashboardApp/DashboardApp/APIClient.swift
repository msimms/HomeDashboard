//
//  APIClient.swift
//  Created by Michael Simms on 2/7/26.
//

import Foundation

struct StatusDTO: Decodable {
	let message: String
	let value: Int
}

enum APIError: Error { case badURL, badResponse }

struct APIClient {
	static func fetchStatus() async throws -> StatusDTO {
		guard let url = URL(string: "https://status.mikesimms.net/api/1.0/indoor_air?latest") else { throw APIError.badURL }
		let (data, response) = try await URLSession.shared.data(from: url)
		guard let http = response as? HTTPURLResponse, (200..<300).contains(http.statusCode) else {
			throw APIError.badResponse
		}
		return try JSONDecoder().decode(StatusDTO.self, from: data)
	}
}
