#include "MenuGroupBuild.hxx"
#include "../../basics/log.hxx"
#include "../../basics/settings.hxx"
#include "../../tileManager.hxx"
#include "../../basics/signal.hxx"

void MenuGroupBuild::draw() const
{
  for (const auto &it : m_groupElements)
  {
    if (it->isVisible())
    {
      it->draw();
    }
  }
  for (auto it : m_buildSubMenuGroups)
  {
    for (auto uielement : it.second->getAllButtons())
    {
      if (uielement->isVisible())
      {
        uielement->draw();
      }
    }
  }
}

void MenuGroupBuild::constructMenu()
{
  std::string stringToCut = "_sub";
  std::string::size_type sizeOfStringToCut;
  std::string newString;

  for (auto it : m_groupElements)
  {
    // TODO: replace by an enum, when BetterEnums is added.
    // TODO: parse all non - _sub elements first, to make sure the main group exists.
    if (!it->getUiElementData().menuGroupID.empty())
    {
      sizeOfStringToCut = it->getUiElementData().menuGroupID.find("_sub");
      // check if it the button is a submenu
      if (it->getUiElementData().menuGroupID.find(stringToCut) == std::string::npos)
      {
        // create an empty ButtonGroup in the m_buildSubmenuGroups, so we know that elements can be placed here
        m_buildSubMenuGroups[it->getUiElementData().menuGroupID] = new ButtonGroup;
        m_buildMenuGroup->addToGroup(it);
      }
      else
      {
        newString = it->getUiElementData().menuGroupID;

        newString.erase(sizeOfStringToCut, stringToCut.size());
        if (m_buildSubMenuGroups.find(newString) != m_buildSubMenuGroups.end())
        {
          m_buildSubMenuGroups[newString]->addToGroup(it);
        }
        else
        {
          LOG(LOG_ERROR) << "You are trying to add an UiElement to the Group " << newString
                         << ", but the main group does not exist.";
        }
      }
    }
  }

  for (auto it : m_buildMenuGroup->getAllButtons())
  {
    // check if there's a corresponding category for tiles for this menu ID.
    for (auto &tile : TileManager::instance().getAllTileData())
    {
      if (tile.second.category == it->getUiElementData().menuGroupID)
      {
        //TODO: take care of ownership / add the created elements to UiManager to properly delete them
        Button *button = new Button({0, 0, 0, 0});

        // TODO: Check if icon empty.
        button->setTextureID("Button_NoIcon");
        button->drawImageButtonFrame(true);
        button->setVisibility(false);
        button->setToggleButton(true);

        button->setActionID("ChangeTileType");
        button->setActionParameter(tile.first);

        if (m_buildSubMenuGroups[tile.second.category])
        {
          m_buildSubMenuGroups[tile.second.category]->addToGroup(button);
        }
        else
        {
          LOG(LOG_ERROR) << "Attempting to add element " << tile.first << " to category " << tile.second.category
                         << ". But the Category doesn't exist.";
        }
        LOG() << "Found matching category " << tile.first;
      }
    }
  }

  // set actionID
  for (auto it : m_buildMenuGroup->getAllButtons())
  {
    if (m_buildSubMenuGroups.count(it->getUiElementData().menuGroupID))
    {
      it->setActionID("ToggleVisibilityOfGroup");
      it->setActionParameter(it->getUiElementData().menuGroupID);
      LOG() << "Adding action for: " << it->getUiElementData().menuGroupID;
    }
  }

  arrangeElements();
}

void MenuGroupBuild::arrangeElements()
{
  SDL_Point screenCenter{Settings::instance().settings.screenWidth / 2, Settings::instance().settings.screenHeight / 2};
  int mainGroupWidth = 0;

  //TODO: Make Padding an attribute
  int padding = 16;
  int paddingVertical = 16;
  int subMenuPadding = 8;
  int subMenuPaddingVertical = 8;

  // get width for all main elements
  for (auto it : m_buildMenuGroup->getAllButtons())
  {
    mainGroupWidth += it->getUiElementRect().w;
  }
  // add padding between main elements to width
  mainGroupWidth -= static_cast<int>(padding * (m_buildMenuGroup->count() - 1));

  // calculate x starting point
  int xOffset = screenCenter.x - mainGroupWidth;
  int currentElement = 1;

  // set position for all main elements
  for (auto it : m_buildMenuGroup->getAllButtons())
  {
    int elementWidth = it->getUiElementRect().w;
    if (!it->getUiElementData().menuGroupID.empty())
    {
      int x = static_cast<int>(xOffset + (elementWidth * currentElement) + padding * (currentElement - 1));
      int y = Settings::instance().settings.screenHeight - it->getUiElementRect().h - paddingVertical;
      it->setPosition(x, y);
      currentElement++;
    }

    // loop for re-arranging buttons that are in subgroups.
    if (m_buildSubMenuGroups.find(it->getUiElementData().menuGroupID) != m_buildSubMenuGroups.end())
    {
      int subMenuGroupWidth = 0;
      SDL_Rect parentRect = it->getUiElementRect();

      // calculate width for all elements of submenu group
      for (auto groupElement : m_buildSubMenuGroups[it->getUiElementData().menuGroupID]->getAllButtons())
      {
        subMenuGroupWidth += groupElement->getUiElementRect().w;
      }
      // add total size of padding between elements
      subMenuGroupWidth +=
          static_cast<int>(subMenuPadding * (m_buildSubMenuGroups[it->getUiElementData().menuGroupID]->count() - 1));

      int subMenuXOffset = parentRect.x - subMenuGroupWidth / 2;
      int subMenuYOffset = parentRect.y;

      int currentSubElement = 1;
      for (auto groupElement : m_buildSubMenuGroups[it->getUiElementData().menuGroupID]->getAllButtons())
      {
        int x =
            subMenuXOffset + groupElement->getUiElementRect().w * currentSubElement + subMenuPadding * (currentSubElement - 1);
        int y = subMenuYOffset - groupElement->getUiElementRect().h - subMenuPaddingVertical;
        groupElement->setPosition(x, y);
        currentSubElement++;
      }
    }
  }
}