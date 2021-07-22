#include <sdk.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <netcode.hpp>

struct PlayerMenuData final : IPlayerMenuData {
	uint8_t menuId = INVALID_MENU_ID;

	uint8_t getMenuID() const override {
		return menuId;
	}

	void setMenuID(uint8_t id) override {
		menuId = id;
	}

	void free() override {
		delete this;
	}
};

struct Menu final : public IMenu, public PoolIDProvider, public NoCopy {
	String title;
	uint8_t columnCount;
	Vector2 pos;
	float col1Width;
	float col2Width;
	bool menuEnabled;
	StaticArray<bool, MAX_MENU_ITEMS> rowEnabled;
	StaticArray<String, 2> columnHeaders;
	StaticArray<uint8_t, 2> columnItemCount;
	StaticArray<StaticArray<String, MAX_MENU_ITEMS>, 2> columnMenuItems;

	UniqueIDArray<IPlayer, IPlayerPool::Cnt> initedFor_;

	void setColumnHeader(StringView header, MenuColumn column) override {
		if (column > columnCount) {
			return;
		}

		columnHeaders.at(column) = header;
	}

	void addMenuItem(StringView itemText, MenuColumn column) override {
		if (column > columnCount)
		{
			return;
		}

		uint8_t itemCount = columnItemCount.at(column);

		if (itemCount >= MAX_MENU_ITEMS)
		{
			return;
		}

		columnMenuItems.at(column).at(itemCount) = itemText;
		itemCount++;
		columnItemCount.at(column) = itemCount;

		initedFor_.clear();
	}

	void disableMenuRow(MenuRow row) override {
		if (row >= MAX_MENU_ITEMS)
		{
			return;
		}

		rowEnabled.at(row) = false;
		initedFor_.clear();
	}

	void disableMenu() override {
		menuEnabled = false;
		initedFor_.clear();
	}

	void initForPlayer(IPlayer & player) override {
		NetCode::RPC::PlayerInitMenu playerInitMenu;
		playerInitMenu.MenuID = poolID;
		playerInitMenu.HasTwoColumns = columnCount >= 1;
		playerInitMenu.Title = StringView(title);
		playerInitMenu.Position = pos;
		playerInitMenu.Col1Width = col1Width;
		playerInitMenu.Col2Width = col2Width;
		playerInitMenu.MenuEnabled = menuEnabled;
		playerInitMenu.RowEnabled = rowEnabled;

		for (int i = 0; i < columnHeaders.size(); i++) {
			playerInitMenu.ColumnHeaders.at(i) = StringView(columnHeaders.at(i));
		}

		playerInitMenu.ColumnItemCount = columnItemCount;

		for (int column = 0; column < columnMenuItems.size(); column++) {
			StaticArray<String, MAX_MENU_ITEMS> singleColumnItems = columnMenuItems.at(column);
			for (int row = 0; row < singleColumnItems.size(); row++) {
				playerInitMenu.MenuItems.at(column).at(row) = StringView(singleColumnItems.at(row));
			}
		}

		playerInitMenu.MenuItems[0] = {
			StringView(columnMenuItems[0][0]), StringView(columnMenuItems[0][1]), StringView(columnMenuItems[0][2]),
			StringView(columnMenuItems[0][3]), StringView(columnMenuItems[0][4]), StringView(columnMenuItems[0][5]),
			StringView(columnMenuItems[0][6]), StringView(columnMenuItems[0][7]), StringView(columnMenuItems[0][8]),
			StringView(columnMenuItems[0][9]), StringView(columnMenuItems[0][10]), StringView(columnMenuItems[0][11])
		};

		playerInitMenu.MenuItems[1] = {
			StringView(columnMenuItems[1][0]), StringView(columnMenuItems[1][1]), StringView(columnMenuItems[1][2]),
			StringView(columnMenuItems[1][3]), StringView(columnMenuItems[1][4]), StringView(columnMenuItems[1][5]),
			StringView(columnMenuItems[1][6]), StringView(columnMenuItems[1][7]), StringView(columnMenuItems[1][8]),
			StringView(columnMenuItems[1][9]), StringView(columnMenuItems[1][10]), StringView(columnMenuItems[1][11])
		};

		player.sendRPC(playerInitMenu);
		initedFor_.add(player.getID(), player);
	}

	void showForPlayer(IPlayer & player) override {
		if (!initedFor_.valid(player.getID())) {
			initForPlayer(player);
		}
		NetCode::RPC::PlayerShowMenu playerShowMenu;
		playerShowMenu.MenuID = poolID;
		player.sendRPC(playerShowMenu);

		IPlayerMenuData * data = player.queryData<IPlayerMenuData>();
		data->setMenuID(poolID);
	}

	void hideForPlayer(IPlayer & player) override {
		NetCode::RPC::PlayerHideMenu playerHideMenu;
		playerHideMenu.MenuID = poolID;
		player.sendRPC(playerHideMenu);

		IPlayerMenuData * data = player.queryData<IPlayerMenuData>();
		data->setMenuID(INVALID_MENU_ID);
	}

	void resetForPlayer(IPlayer & player) override {
		if (initedFor_.valid(player.getID())) {
			initedFor_.remove(player.getID(), player);
		}
	}

	int getID() const override {
		return poolID;
	}

	~Menu() {

	}
};