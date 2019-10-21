#!/usr/bin/python
# -*- coding: utf-8 -*-

def connectNearest(facility_list, client_list, open_facilities):

	solution = []
	for j in range(0, len(client_list)):
		min_facility = -1
		min_dist = -1
		for i in range(0, len(open_facilities)):
			current_facility = open_facilities[i]
			current_dist = client_list[j].attribution_cost[current_facility]

			if (min_dist == -1 or current_dist < min_dist):
				min_facility = current_facility
				min_dist = current_dist
		solution.append(min_facility)

	return solution
