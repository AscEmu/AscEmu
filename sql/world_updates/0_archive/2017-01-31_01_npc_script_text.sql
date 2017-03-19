--
-- script text for pit of saron
--
REPLACE INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `type`, `sound`) VALUES
  (8762, 'That one maybe not so good to eat now, stupied Garfrost! BAD! BAD!', 36494, 14, 16914),
  (8763, 'Will save.. for snack for.. for later!', 36494, 14, 16913),
  (8764, 'Garfrost hope giant underpants clean. Save boss great shame. For later.', 36494, 14, 16915),
  (8765, 'Axe too weak. Garfrost make better weapon and crush you!', 36494, 14, 16916),
  (8766, 'Garfrost tired of puny mortals, soon your bones will FREEZE!', 36494, 14, 16917),
  (8767, 'Our work must not be interrupted! Ick! Take care of them!', 36477, 14, 16926),
  (8768, 'Ooh...We could probably use these parts!', 36477, 14, 16927),
  (8769, 'Arms and legs are in short supply...Thanks for your contribution!', 36477, 14, 16928),
  (8770, 'Quickly! Poison them all while they\'re still close!', 36477, 14, 16930),
  (8771, 'No! That one! That one! Get that one!', 36477, 14, 16931),
  (8772, 'I\'ve changed my mind...go get that one instead!', 36477, 14, 16932),
  (8773, 'What are you attacking him for? The dangerous one is over there,fool!', 36477, 14, 16933),
  (8774, 'Enough moving around! Hold still while I blow them all up!', 36477, 14, 16929),
  (8775, 'Wait! Stop! Don\'t kill me, please! I\'ll tell you everything!', 36477, 14, 16934),
  (8776, 'I\'m not so naive as to believe your appeal for clemency, but I will listen.', 36993, 14, 16611),
  (8777, 'Why should the Banshee Queen spare your miserable life?', 36990, 14, 17033),
  (8778, 'What you seek is in the master\'s lair, but you must destroy Tyrannus to gain entry. Within the Halls of Reflection you will find Frostmourne. It... it holds the truth.', 36477, 14, 16935),
  (8779, 'Frostmourne lies unguarded? Impossible!', 36993, 14, 16612),
  (8780, 'Frostmourne? The Lich King is never without his blade! If you are lying to me...', 36990, 14, 17034),
  (8781, 'I swear it is true! Please, don\'t kill me!!', 36477, 14, 16936),
  (8782, 'Worthless gnat! Death is all that awaits you!', 36477, 14, 16753),
  (8783, 'Urg... no!!', 36477, 14, 16937),
  (8784, 'Do not think that I shall permit you entry into my master\'s sanctum so easily. Pursue me if you dare.', 36477, 14, 16754),
  (8785, 'What a cruel end. Come, heroes. We must see if the gnome\'s story is true. If we can separate Arthas from Frostmourne, we might have a chance at stopping him.', 36993, 14, 16613),
  (8786, 'A fitting end for a traitor. Come, we must free the slaves and see what is within the Lich King\'s chamber for ourselves.', 36990, 14, 17035);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate`='2017-01-31_01_npc_script_text' WHERE  `LastUpdate`='2017-01-30_02_misc';
