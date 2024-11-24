# XAC Lottery

活動廠商提供的抽獎程式每次要收一萬元以上, 操刀看看可不可以自己賺

# Build 

Cmake + SDL(FetchContent) + SDL_image(FetchContent)

# 行為

- 背景圖固定讀取`asset\\background.png`
- 翻面的材質固定讀取`asset\\0021-1024x1024.jpg`
- 抽獎候選者的圖片可以用`.jpg` `.png`, 固定放在`asset\\candidates`資料夾內, 建議使用工號當檔名, log中可以回顧是那些工號中獎
- log檔會產生在`log`資料夾內
- 同一個session內, 被抽中的圖片會被暫時從名單中移除, 不會重複中獎
- 亂數使用`c++11 <random>`
- 按`Enter`開始抽獎, 中獎畫面按`Enter`回到idle狀態, 按`Esc`退出

