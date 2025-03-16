//
//  Database.swift
//  Created by Michael Simms on 2/22/25.
//

import Foundation
import MongoSwift
import NIO

class Database {
	private var client: MongoClient?
	private var database: MongoDatabase?
	
	/// Attempts to connect to the database with the given URL and select the database with the given name.
	func connect(url: String, db_name: String) async {
		do {
			let eventLoopGroup = MultiThreadedEventLoopGroup(numberOfThreads: 1)
			
			// Connect to the database.
			self.client = try MongoClient(url, using: eventLoopGroup)
			if let client = self.client {
				
				// Select the database.
				self.database = client.db(db_name)
			}
		} catch {
			print("Failed to connect to MongoDB: \(error)")
		}
	}
	
	func listCollections() async -> [String] {
		do {
			// Make sure we have a database connection.
			if let db = self.database {
				let collections = try await db.listCollectionNames()
				return collections
			}
		}
		catch {
			NSLog(error.localizedDescription)
		}
		return []
	}

	// Function to fetch documents with timestamp greater than a given value
	func fetchDocumentsWithTimestampAtLeast(collection_name: String, timestamp: Int, handler: (BSONDocument) -> Void) async {
		do {
			// Make sure we have a database connection.
			if let db = self.database {

				// Get a handle to the collection.
				let collection = db.collection(collection_name)

				// Query for documents where "timestamp" is greater than user-defined value
				let query: BSONDocument = ["ts": ["$gt": BSON(timestamp)]]

				// Execute the query and iterate over results
				let cursor = try await collection.find(query)

				for try await document in cursor {
					handler(document)
				}
			}
		} catch {
			print("Error fetching documents: \(error)")
		}
	}
}
