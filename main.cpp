#include <iostream>
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/termcolor.hpp>
#include <vector>
#include <thread>
#include <mutex>
#include <curl/curl.h>
#include <indicators/multi_progress.hpp>
#include <indicators/cursor_control.hpp>

std::mutex progressMutex;
using namespace indicators;

// Declare DynamicProgress<ProgressBar> bars as a global variable
DynamicProgress<ProgressBar> bars;

// Function to generate a random color
// Function to generate a random color
auto getRandomColor()
{
    static const indicators::Color colors[] = {
        indicators::Color::red,
        indicators::Color::green,
        indicators::Color::blue,
        indicators::Color::yellow,
        indicators::Color::cyan,
        indicators::Color::magenta};

    // Seed random number generator
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }

    // Get the size of the colors array
    constexpr size_t numColors = 5;

    // Generate a random index within the range of the colors array
    size_t randomIndex = std::rand() % numColors;

    // Return the color at the random index
    return colors[randomIndex];
}

// Function to create a progress bar with specified color and name
ProgressBar &createProgressBar(const std::string &name, indicators::Color color)
{
    return *(new ProgressBar(
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"="},
        option::Lead{">"},
        option::Remainder{" "},
        option::End{" ]"},
        option::ForegroundColor{color},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true},
        option::PrefixText{name + " "},
        option::FontStyles{
            std::vector<FontStyle>{FontStyle::bold}}));
}

// Progress callback function
int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    // Calculate progress percentage
    double progress = (dlnow / dltotal) * 100.0;

    // Retrieve file name
    int index = *static_cast<int *>(clientp);
    // std::lock_guard<std::mutex> lock(progressMutex);

    while (!bars[index].is_completed())
    {
        bars[index].tick();
    }

    return 0;

    // std::cout << "Downloading " << *fileName << ": " << progress << "%" << std::endl;
}

// Function to download a file given its URL
void downloadFile(const std::string &url, const std::string &fileName, int index)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        FILE *file = fopen(fileName.c_str(), "wb"); // Specify the filename you want to save the downloaded file
        if (file)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on HTTP errors

            // Set progress callback function
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &index); // Pass progress bar as progress data
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cerr << "Error downloading file from " << url << ": " << curl_easy_strerror(res) << std::endl;
            }
            fclose(file);
        }
        else
        {
            std::cerr << "Error creating file for download." << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "Error initializing libcurl." << std::endl;
    }
}

int main()
{
    show_console_cursor(false);

    // List of URLs and corresponding file names to download concurrently
    std::vector<std::pair<std::string, std::string>> downloads = {
        {"https://futu-re.co.jp/wp-content/uploads/2023/02/LF1.jpg", "lift-security-patch.exe"},
        {"https://futu-re.co.jp/wp-content/uploads/2023/02/SC1-1024x724.png", "lift-lights-patch.exe"},
        {"https://futu-re.co.jp/wp-content/uploads/2023/02/futu-reooie%C2%B5ae-7-2048x1535.jpg", "lift-power-patch.exe"}};

    std::cout << "Downloading Firmware updates for Autonomous Lifter:\n";

    // Vector to hold threads
    std::vector<std::thread> threads;

    // Vector to hold progress bars
    std::vector<std::unique_ptr<ProgressBar>> progressBars;

    // Create progress bars and assign to vector
    for (size_t i = 0; i < downloads.size(); ++i)
    {
        bars.push_back(createProgressBar(downloads[i].second, getRandomColor()));
    }

    // Create threads for downloading each file
    for (size_t i = 0; i < downloads.size(); ++i)
    {
        threads.emplace_back(downloadFile, downloads[i].first, downloads[i].second, i);
    }
    // Join threads
    for (auto &thread : threads)
    {
        thread.join();
    }

    std::cout << "Downloads and installtion completed." << std::endl;
    std::cin.get();

    return 0;
}