#pragma once // (c) 2020 Lukas Brunner

#include "Texture.h"
#include "Common/BoundingBox.h"

BRWL_RENDERER_NS


//!  A class representing a dataset.
/*!
 * The DataSet class is a Texture augmented with abilities to load data from files on disk.
 */
template<SampleFormat S>
class DataSet : public Texture<S>
{
public:
	
	DataSet(const BRWL_CHAR* name) : Texture<S>(name),
		sourcePath(),
		bbox({}, {})
	{ }

	/*!
	 * Loads raw data from a given file.
	 * \param
	 */
	void loadFromFile(const BRWL_CHAR* relativePath);

	/**
	 * Returns the path to the file last loaded via DataSet::loadFromFile.
	 * 
	 * \return The path to the file.
	 */
	const BRWL_CHAR* getSourcePath() const { return sourcePath.c_str(); }

	/**
	 * Retrieve the bounding box of this DataSet.
	 * 
	 * \return The bounding box of this DataSet centered around the origin in pixel coordinates.
	 */
	const ::BRWL::BBox& getBoundingBox() const { return bbox; }

protected:

	BRWL_STR sourcePath;
	::BRWL::BBox bbox;
};

#define BRWL_TYPEDEF_(format) typedef DataSet<SampleFormat::format> DataSet ## format;
BRWL_FOR_ALL_SAMPLE_FORMATS(BRWL_TYPEDEF_)
#undef BRWL_TYPEDEF_



BRWL_RENDERER_NS_END