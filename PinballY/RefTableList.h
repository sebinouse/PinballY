// This file is part of PinballY
// Copyright 2018 Michael J Roberts | GPL v3 or later | NO WARRANTY
//
// Reference Table List.  This object maintains a master list of 
// real pinball machines that we read from an external data file.
// We use this for populating pick lists in the game setup dialog,
// to make data entry easier by pre-filling fields with reference
// data from our master list.  We use fuzzy matching to find 
// likely tables from the master list based on the filename of
// the new game we're setting up.
//
// Note that the table is loaded into memory asynchronously from
// the main UI, in a background thread.  (We do this because it's
// quite large, about 6200 tables, and isn't needed until the user
// navigates somewhere in the UI that consumes the data.)  Any
// code that accesses the table data needs to be aware of the
// delayed loading so that it can tolerate a situation where the
// table isn't loaded yet.  Callers shouldn't wait for the load
// to complete; they should instead gracefully degrade and act
// as though the table simply isn't available, as that's always
// a possibility as well (e.g., the user could accidentally 
// delete the file).

#pragma once
#include "CSVFile.h"
#include "DiceCoefficient.h"

class RefTableList
{
public:
	RefTableList();
	~RefTableList();

	// Initialize.  This starts the initializer thread to load
	// the table in the background.
	void Init();

	// Table match results struct
	struct Table
	{
		Table(RefTableList *rtl, int row, float score);

		TSTRING listName;     // list name - "title (manufacturer year)"
		TSTRING name;		  // table name
		TSTRING manuf;		  // manufacturer
		int year;			  // year; 0 if unknown
		int players;		  // number of players; 0 if unknown
		TSTRING themes;		  // themes, with " - " delimiters
		TSTRING sortKey;      // sort key
		TSTRING machineType;  // IPDB machine type code (SS, EM, ME)
		float score;		  // Dice coefficient score for the fuzzy match
	};

	// Get the top N matches to a given string.  The results
	// are sorted by table name.
	void GetTopMatches(const TCHAR *name, int n, std::list<Table> &lst);

protected:
	// Is the table ready?  This checks to see if the initializer
	// thread has finished.
	bool IsReady() const 
	{ 
		return hInitThread != NULL 
			&& WaitForSingleObject(hInitThread, 0) == WAIT_OBJECT_0; 
	}

	// Loader thread handle.  Because of the large data set (about
	// 6200 tables), we load the file in a background thread.  We
	// keep a handle to the thread here so that we can check for
	// completion.
	HandleHolder hInitThread;

	// underlying CSV file data
	CSVFile csvFile;

	// Bigram sets for the Name, AltName, and Initials fields.  The 
	// table is static, so we just build these vectors in parallel, 
	// indexed by the row numbers in the CSV file data.
	std::vector<DiceCoefficient::BigramSet<TCHAR>> nameBigrams;
	std::vector<DiceCoefficient::BigramSet<TCHAR>> altNameBigrams;

	// CSV file column accessors
	CSVFile::Column *nameCol;
	CSVFile::Column *altNameCol;
	CSVFile::Column *manufCol;
	CSVFile::Column *yearCol;
	CSVFile::Column *playersCol;
	CSVFile::Column *typeCol;
	CSVFile::Column *themeCol;

	// Sorting key.  This isn't in the CSV file; it's a column we
	// add to the in-memory set, synthesized from the file data.
	CSVFile::Column *sortKeyCol;

	// List name column.  This is another synthesized column, with
	// the name to show in the drop list, formatted as "Title
	// (Manufacturer Year)"
	CSVFile::Column *listNameCol;

	// Initials column.  This is another synthesized column, with
	// the initials of the game's name.  Many filenames refer to 
	// the title by its initials instead of using the full name,
	// to keep the filename compact.  We don't attempt a bigram
	// match on this because that yields too many false positives
	// with such short strings; we just do a plain substring 
	// search instead.
	CSVFile::Column *initialsCol;

	// create the sorting key for a row
	void MakeSortKey(int row);

	// create the list name for a row
	void MakeListName(int row);
};
