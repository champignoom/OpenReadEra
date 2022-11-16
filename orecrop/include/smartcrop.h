/*
 * Copyright (C) 2013 The Common CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#ifndef _ORE_CROP_H_
#define _ORE_CROP_H_

#include <stdint.h>
#include <array>
#include <vector>
#include <string>

constexpr int smart_crop_w = 400;
constexpr int smart_crop_h = 400;

void CalcBitmapSmartCrop(float* crop, uint8_t* pixels, int w, int h, float slice_l, float slice_t, float slice_r, float slice_b);

/*
class CropTile
{
private:
	const uint8_t *pixels;
public:
	int pixels_w;
	int pixels_h;

	int x;
	int y;
	int w;
	int h;
	int lum = -1;
	float colored = -1;
	float light   = -1;
	float dark    = -1;

	int arr_x, arr_y;

	CropTile(const uint8_t *pixels,int p_w,int p_h, int x, int y, int w, int h, int a_x, int a_y):
	pixels(pixels),pixels_w(p_w),pixels_h(p_h),x(x),y(y),w(w),h(h),arr_x(a_x),arr_y(a_y) {}

	int calcAvgLuminosity()
	{
		if(lum >= 0)
		{
			return lum;
		}

		int i;
		int mid_bright = 0;
		int func_x, func_y;
		for (func_y = 0; func_y < h; func_y++)
		{
			for (func_x = 0; func_x < w; func_x++)
			{
				i = ((func_y + y) * pixels_w + x + func_x) * 4;
				int min_lum = std::min(pixels[i + 2], std::min(pixels[i + 1], pixels[i]));
				int max_lum = std::max(pixels[i + 2], std::max(pixels[i + 1], pixels[i]));
				mid_bright += (min_lum + max_lum) / 2;
			}
		}
		mid_bright /= (w * h);
		lum = mid_bright;
		return mid_bright;
	}

	void calcPixStat()
	{
		int i;
		int func_x, func_y;

		int colored_pix = 0;
		int dark_pix = 0;
		int light_pix = 0;
		for (func_y = 0; func_y < h; func_y++)
		{
			for (func_x = 0; func_x < w; func_x++)
			{
				i = ((func_y + y) * pixels_w + x + func_x) * 4;
				char r = pixels[i+0];
				char g = pixels[i+1];
				char b = pixels[i+2];
				//char a = pixels[i+3];

				if(r < g - 8 || r > g + 8) {colored_pix++; continue;}
				if(g < b - 8 || g > b + 8) {colored_pix++; continue;}
				if(b < r - 8 || b > r + 8) {colored_pix++; continue;}

				if((r+g+b)/3 > 70)
				{
					light_pix++;
				}
				else
				{
					dark_pix++;
				}
			}
		}

		int size = w*h;
		colored =  (float)colored_pix / (float)size;
		light   =  (float)light_pix   / (float)size;
		dark    =  (float)dark_pix    / (float)size;

		//LE(" tile c_pix = %d",colored_pix);
		//LE(" tile l_pix = %d",light_pix  );
		//LE(" tile d_pix = %d",dark_pix   );
		//LE(" tile c = %f",colored);
		//LE(" tile l = %f",light);
		//LE(" tile d = %f",dark);
	}
};

class CropArray
{
public:


	std::vector<std::vector<CropTile*>> array;
	int arr_w = 0;
	int arr_h = 0;

	CropArray(uint8_t *pixels, int w, int h)
	{
		arr_w = ceil(w/10);
		arr_h = ceil(h/10);

		int tile_w = w / arr_w;
		int tile_h = h / arr_h;
		//LE("arr %d x %d", arr_w,arr_h);
		//LE("tile %d x %d",tile_w, tile_h);
		for (int a_y = 0; a_y < arr_h; a_y++)
		{
			std::vector<CropTile*> line;
			for (int a_x = 0; a_x < arr_w; a_x++)
			{
				auto *tile = new CropTile(pixels, w, h, a_x * tile_w, a_y * tile_h, tile_w, tile_h, a_x, a_y);
				line.push_back(tile);
			}
			array.push_back(line);
		}
	};

	CropTile* at(int x, int y) {return array.at(y).at(x);}

	int getLumForZone(int x, int y, int w, int h)
	{
		if(x < 0 || y < 0 || x >= arr_w || y >= arr_h)
		{
			return 0;
		}
		if(w < 0 || h < 0 || w > arr_w || h > arr_h)
		{
			return 0;
		}
		if(x + w > arr_w || y + h > arr_h)
		{
			return 0;
		}
		//LE("getLumForZone x %d-%d ; y %d-%d ", x, x+w, y, y+h);

		int lum = 0;
		int tilecount = 0;

		for (int y_ = y; y_ < y + h; y_++)
		{
			for (int x_ = x; x_ < x + w; x_++)
			{
				auto *tile = this->at(x_,y_);

				lum += tile->calcAvgLuminosity();
				tilecount++;
				//LE("tile %d x %d lum = %d, lum = %d, tilecount = %d", x_, y_, tile->lum, lum, tilecount);
			}
		}
		if(tilecount == 0)
		{
			return 0;
		}
		return lum / tilecount;
	}

	void calcPixStat(float *c,float *l,float *d)
	{
		calcPixStatForZone(0, 0, arr_w, arr_h, c, l ,d);
	}

	void calcPixStatForZone(int x, int y, int w, int h, float *c,float *l,float *d)
	{
        //LE("calcPixStatForZone x %d-%d ; y %d-%d , arr = %d X %d", x, x+w, y, y+h,arr_w, arr_h);
		if(x < 0 || y < 0 || x >= arr_w || y >= arr_h)
		{
            LE("err 1");
			return;
		}
		if(w < 0 || h < 0 || w > arr_w || h > arr_h)
		{
            LE("err 2");
			return;
		}
		if(x + w > arr_w || y + h > arr_h)
		{
            LE("err 3");
			return;
		}

		float coloredCount = 0;
		float darkCount = 0;
		float lightCount = 0;
		int tilecount = 0;

		for (int y_ = y; y_ < y + h; y_++)
		{
			for (int x_ = x; x_ < x + w; x_++)
			{
				auto *tile = this->at(x_,y_);
				//LE("tile %d x %d tilecount = %d", x_, y_, coloredCount, tilecount);

				tile->calcPixStat();

				coloredCount += tile->colored;
				lightCount   += tile->light;
				darkCount    += tile->dark;

				tilecount++;
			}
		}
		if(tilecount == 0)
		{
			return;
			LE("tilecount == 0");
		}
		*c = (float)coloredCount / (float)tilecount;
		*l = (float)lightCount   / (float)tilecount;
		*d = (float)darkCount    / (float)tilecount;
		}


	void free_arr()
	{
		//LE("free arr %d %d",arr_w,arr_h);
		if(array.empty())
			return;
		for (int y = 0; y < arr_h; y++)
		{
			if(array.at(y).empty())
				return;

			for (int x = 0; x < arr_w; x++)
			{
				//LE("    free_arr x = %d, y = %d",x,y);
				free(array.at(y).at(x));
			}
		}
	}

	~CropArray()
	{
		free_arr();
	}

	//printOut luminocity values for all tiles
    void printOutLum()
    {
        for (int y = 0; y < arr_h; y++)
        {
            std::string rowstr;
            for (int x = 0; x < arr_w; x++)
            {
                auto *tile = at(x,y);
                rowstr += " " + std::to_string(tile->calcAvgLuminosity());
            }
            //LW(" %s",rowstr.c_str());
        }
    }
};
*/
#endif //_ORE_CROP_H_
